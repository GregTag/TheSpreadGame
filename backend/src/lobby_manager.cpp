#include "lobby_manager.hpp"

#include <spdlog/spdlog.h>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/experimental/cancellation_condition.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <sstream>

#include "errors.hpp"
#include "session.hpp"

boost::asio::awaitable<std::string> LobbyManager::Connect(
    std::shared_ptr<Session> session) {
  return boost::asio::co_spawn(strand_, ConnectImpl(std::move(session)),
                               boost::asio::use_awaitable);
}

boost::asio::awaitable<std::string> LobbyManager::ConnectImpl(
    std::shared_ptr<Session> session) {
  std::ostringstream oss;
  oss << "p" << player_counter_++;
  std::string pid = oss.str();
  sessions_[pid] = session;
  spdlog::info("Registered session for {}", pid);
  co_return pid;
}

boost::asio::awaitable<void> LobbyManager::Disconnect(
    const std::string& player_id) {
  return boost::asio::co_spawn(strand_, DisconnectImpl(player_id),
                               boost::asio::use_awaitable);
}

boost::asio::awaitable<void> LobbyManager::DisconnectImpl(
    const std::string& player_id) {
  sessions_.erase(player_id);
  // If in a lobby, remove the player and broadcast update
  spdlog::info("Disconnect {}", player_id);
  co_await LeaveLobbyImpl(player_id);
}

boost::asio::awaitable<std::string> LobbyManager::CreateLobby(
    const std::string& player_id, models::LobbyOptions options) {
  return boost::asio::co_spawn(strand_, CreateLobbyImpl(player_id, options),
                               boost::asio::use_awaitable);
}

boost::asio::awaitable<std::string> LobbyManager::CreateLobbyImpl(
    const std::string& player_id, models::LobbyOptions options) {
  if (membership_.find(player_id) != membership_.end()) {
    spdlog::warn("{} already in a lobby", player_id);
    throw errors::kPlayerAlreadyInLobby;
  }

  std::ostringstream oss;
  oss << "l" << lobby_counter_++;
  std::string lobby_id = oss.str();
  lobbies_[lobby_id] =
      models::Lobby{lobby_id, player_id, {player_id}, std::move(options)};
  membership_[player_id] = lobby_id;
  spdlog::info("Created lobby {} by {}", lobby_id, player_id);
  SendToAll({{"type", "lobby_created"}, {"lobby", lobbies_[lobby_id]}});
  co_return lobby_id;
}

boost::asio::awaitable<void> LobbyManager::JoinLobby(
    const std::string& lobby_id, const std::string& player_id) {
  return boost::asio::co_spawn(strand_, JoinLobbyImpl(lobby_id, player_id),
                               boost::asio::use_awaitable);
}

boost::asio::awaitable<void> LobbyManager::JoinLobbyImpl(
    const std::string& lobby_id, const std::string& player_id) {
  auto it = lobbies_.find(lobby_id);
  if (it == lobbies_.end()) {
    spdlog::warn("lobby {} not found for join by {}", lobby_id, player_id);
    throw errors::kLobbyNotFound;
  }
  if (membership_.find(player_id) != membership_.end()) {
    spdlog::warn("{} already in a lobby on join {}", player_id, lobby_id);
    throw errors::kPlayerAlreadyInLobby;
  }
  models::Lobby& lobby = it->second;
  if (lobby.status != models::LobbyStatus::Open) {
    spdlog::warn("{} tried to join non-open lobby {}", player_id, lobby_id);
    throw errors::kGameAlreadyStarted;
  }
  if (static_cast<int>(lobby.players.size()) >= lobby.options.max_players) {
    spdlog::warn("{} tried to join full lobby {}", player_id, lobby_id);
    throw errors::kLobbyFull;
  }

  lobby.players.push_back(player_id);
  membership_[player_id] = lobby_id;
  SendToAll({{"type", "lobby_update"}, {"lobby", lobby}});
  co_return;
}

boost::asio::awaitable<void> LobbyManager::LeaveLobby(
    const std::string& player_id) {
  return boost::asio::co_spawn(strand_, LeaveLobbyImpl(player_id),
                               boost::asio::use_awaitable);
}

boost::asio::awaitable<void> LobbyManager::LeaveLobbyImpl(
    const std::string& player_id) {
  auto it = membership_.find(player_id);
  if (it == membership_.end()) {
    spdlog::warn("{} tried to leave but not in lobby", player_id);
    throw errors::kPlayerNotInLobby;
  }

  auto lobby_id = it->second;
  membership_.erase(it);

  auto lit = lobbies_.find(lobby_id);
  if (lit == lobbies_.end()) {
    spdlog::warn("lobby {} vanished before leave of {}", lobby_id, player_id);
    throw errors::kLobbyNotFound;
  }

  models::Lobby& lobby = lit->second;

  // If the lobby is in progress, eliminate the player from the game
  if (lobby.status == models::LobbyStatus::InProgress) {
    if (auto game = games_[lobby_id].lock()) {
      game->EliminatePlayer(player_id);
    }
  }

  auto pos = std::ranges::find(lobby.players, player_id);
  lobby.players.erase(pos);

  if (lobby.players.empty()) {
    spdlog::info("Removed last player {}; deleting lobby {}", player_id,
                 lobby_id);
    lobbies_.erase(lit);
    SendToAll({{"type", "lobby_gone"}, {"lobby_id", lobby_id}});
    co_return;
  }
  if (lobby.host_player_id == player_id) {
    // reassign host if possible
    lobby.host_player_id = lobby.players.front();
  }
  SendToAll({{"type", "lobby_update"}, {"lobby", lobby}});
}

boost::asio::awaitable<nlohmann::json> LobbyManager::ListLobbies() const {
  return boost::asio::co_spawn(strand_, ListLobbiesImpl(),
                               boost::asio::use_awaitable);
}

boost::asio::awaitable<nlohmann::json> LobbyManager::ListLobbiesImpl() const {
  nlohmann::json result = nlohmann::json::array();
  for (const auto& [_, lobby] : lobbies_) {
    result.push_back(lobby);
  }
  co_return result;
}

void LobbyManager::SendToAll(nlohmann::json msg) {
  auto data = std::make_shared<std::string>(msg.dump());

  for (const auto& [id, session] : sessions_) {
    if (auto s = session.lock()) {
      s->Send(data);
    }
  }
}

void LobbyManager::SendToLobby(const std::string& lobby_id,
                               nlohmann::json msg) {
  auto it = lobbies_.find(lobby_id);
  if (it == lobbies_.end()) {
    return;
  }

  auto data = std::make_shared<std::string>(msg.dump());

  for (auto const& p : it->second.players) {
    if (auto s = sessions_[p].lock()) {
      s->Send(data);
    }
  }
}

boost::asio::awaitable<void> LobbyManager::StartGame(
    const std::string& player_id) {
  return boost::asio::co_spawn(strand_, StartGameImpl(player_id),
                               boost::asio::use_awaitable);
}

boost::asio::awaitable<void> LobbyManager::StartGameImpl(
    const std::string& player_id) {
  auto mit = membership_.find(player_id);
  if (mit == membership_.end()) {
    throw errors::kPlayerNotInLobby;
  }
  const auto& lobby_id = mit->second;
  auto it = lobbies_.find(lobby_id);
  if (it == lobbies_.end()) {
    throw errors::kLobbyNotFound;
  }
  auto& lobby = it->second;
  if (lobby.host_player_id != player_id) {
    throw errors::kNotLobbyHost;
  }
  if (lobby.players.size() < 2) {
    throw errors::kNotEnoughPlayers;
  }

  std::vector<std::weak_ptr<Session>> sessions;
  sessions.reserve(lobby.players.size());
  for (const auto& p : lobby.players) {
    if (auto s = sessions_[p].lock()) {
      sessions.emplace_back(std::move(s));
    }
  }
  auto game = GameCoordinator::Create(
      *this, lobby, strand_.get_inner_executor(), std::move(sessions));
  game->BroadcastState();
  games_[lobby_id] = std::move(game);
  lobby.status = models::LobbyStatus::InProgress;
  SendToAll({{"type", "lobby_update"}, {"lobby", lobby}});
  co_return;
}

boost::asio::awaitable<void> LobbyManager::EndGame(
    const std::string& lobby_id) {
  return boost::asio::co_spawn(
      strand_, UpdateStatusImpl(lobby_id, models::LobbyStatus::Finished),
      boost::asio::use_awaitable);
}

boost::asio::awaitable<void> LobbyManager::UpdateStatusImpl(
    const std::string& lobby_id, models::LobbyStatus status) {
  auto it = lobbies_.find(lobby_id);
  if (it == lobbies_.end()) {
    throw errors::kLobbyNotFound;
  }
  auto& lobby = it->second;
  lobby.status = status;
  SendToAll({{"type", "lobby_update"}, {"lobby", lobby}});
  co_return;
}
