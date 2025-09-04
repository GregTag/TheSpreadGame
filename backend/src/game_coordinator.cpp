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
      player_ids_(lobby.players.size()),
      sessions_(std::move(sessions)),
      strand_(exec) {
  for (std::size_t idx = 0; idx < lobby.players.size(); ++idx) {
    player_ids_[lobby.players[idx]] = idx + 1;
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
    const std::string& player_id, std::size_t cell_id) {
  return boost::asio::co_spawn(strand_, MakeMoveImpl(player_id, cell_id),
                               boost::asio::use_awaitable);
}

boost::asio::awaitable<void> GameCoordinator::MakeMoveImpl(
    const std::string& player_id, std::size_t cell_id) {
  auto player_index = player_ids_.at(player_id);
  if (player_index != game_.GetCurrentPlayer()) {
    throw spread_logic::errors::kInvalidMove;
  }
  game_.MakeMove(cell_id);
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
  auto player_index = player_ids_.at(player_id);
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
  SendToGame({
      {"type", "game_state"},
      {"field", game_.GetField()},
      {"alive_players", game_.GetAlivePlayers()},
      {"current_player", game_.GetCurrentPlayer()},
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
