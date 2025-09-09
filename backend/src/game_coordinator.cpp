#include "game_coordinator.hpp"

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>

#include "game.hpp"
#include "lobby_manager.hpp"
#include "session.hpp"

std::shared_ptr<GameCoordinator> GameCoordinator::Create(
    LobbyManager& lobby_manager, const models::Lobby& lobby, ExecutorType exec,
    std::vector<std::weak_ptr<Session>> sessions) {
  auto game = std::make_shared<GameCoordinator>(
      lobby_manager, lobby, std::move(exec), std::move(sessions));
  for (const auto& wptr : game->sessions_) {
    if (auto s = wptr.lock()) {
      s->SetGame(game);
    }
  }
  return game;
}

GameCoordinator::GameCoordinator(LobbyManager& lobby_manager,
                                 const models::Lobby& lobby, ExecutorType exec,
                                 std::vector<std::weak_ptr<Session>> sessions)
    : lobby_manager_(lobby_manager),
      game_(lobby.players.size(), lobby.options.width, lobby.options.height),
      id_(lobby.id),
      players_(lobby.players),
      player_to_idx_(lobby.players.size()),
      sessions_(std::move(sessions)),
      strand_(exec) {
  for (std::size_t idx = 0; idx < lobby.players.size(); ++idx) {
    player_to_idx_[lobby.players[idx]] = idx + 1;
  }
}

void GameCoordinator::SendToGame(nlohmann::json msg) {
  auto data = std::make_shared<std::string>(msg.dump());

  for (auto const& wptr : sessions_) {
    if (auto s = wptr.lock()) {
      s->Send(data);
    }
  }
}

boost::asio::awaitable<void> GameCoordinator::MakeMove(
    const std::string& player_id, std::size_t cell_idx) {
  return boost::asio::co_spawn(strand_, MakeMoveImpl(player_id, cell_idx),
                               boost::asio::use_awaitable);
}

boost::asio::awaitable<void> GameCoordinator::MakeMoveImpl(
    const std::string& player_id, std::size_t cell_idx) {
  auto player_index = player_to_idx_.at(player_id);
  if (player_index != game_.GetCurrentPlayer()) {
    throw spread_logic::errors::kInvalidMove;
  }
  game_.MakeMove(cell_idx);
  co_await BroadcastStateImpl();
  if (game_.GetAlivePlayers().size() <= 1) {
    co_await EndGame();
  }
  co_return;
}

void GameCoordinator::EliminatePlayer(const std::string& player_id) {
  boost::asio::co_spawn(strand_, EliminatePlayerImpl(player_id),
                        boost::asio::detached);
}

boost::asio::awaitable<void> GameCoordinator::EliminatePlayerImpl(
    const std::string& player_id) {
  auto player_index = player_to_idx_.at(player_id);
  game_.EliminatePlayer(player_index);
  co_await BroadcastStateImpl();
  if (game_.GetAlivePlayers().size() <= 1) {
    co_await EndGame();
  }
  co_return;
}

void GameCoordinator::BroadcastState() {
  boost::asio::co_spawn(strand_, BroadcastStateImpl(), boost::asio::detached);
}

boost::asio::awaitable<void> GameCoordinator::BroadcastStateImpl() {
  std::vector<std::string_view> alive_players;
  alive_players.reserve(players_.size());
  for (auto idx : game_.GetAlivePlayers()) {
    alive_players.emplace_back(players_[idx - 1]);
  }
  std::string_view current_player = players_[game_.GetCurrentPlayer() - 1];

  SendToGame({
      {"type", "game_state"},
      {"field", game_.GetField()},
      {"alive_players", std::move(alive_players)},
      {"current_player", current_player},
      {"turn", game_.GetCurrentTurn()},
      {"move_history", game_.GetMoveHistory()},
  });
  co_return;
}

boost::asio::awaitable<void> GameCoordinator::EndGame() {
  co_await lobby_manager_.EndGame(id_);
  for (const auto& wptr : sessions_) {
    if (auto s = wptr.lock()) {
      s->SetGame(nullptr);
    }
  }
  co_return;
}
