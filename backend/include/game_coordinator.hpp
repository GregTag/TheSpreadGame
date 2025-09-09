#pragma once

#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <game.hpp>
#include <memory>
#include <string>
#include <unordered_map>

#include "models.hpp"

// Forward declaration
class Session;
class LobbyManager;

class GameCoordinator : std::enable_shared_from_this<GameCoordinator> {
  using ExecutorType = boost::asio::io_context::executor_type;

 public:
  GameCoordinator(LobbyManager&, const models::Lobby&, ExecutorType,
                  std::vector<std::weak_ptr<Session>>);

  static std::shared_ptr<GameCoordinator> Create(
      LobbyManager& lobby_manager, const models::Lobby& lobby,
      ExecutorType exec, std::vector<std::weak_ptr<Session>> sessions);

  boost::asio::awaitable<void> MakeMove(const std::string& player_id,
                                        std::size_t cell_idx);

  void EliminatePlayer(const std::string& player_id);

  void BroadcastState();

  void SendToGame(nlohmann::json msg);

 private:
  boost::asio::awaitable<void> MakeMoveImpl(const std::string& player_id,
                                            std::size_t cell_idx);

  boost::asio::awaitable<void> EliminatePlayerImpl(
      const std::string& player_id);

  boost::asio::awaitable<void> BroadcastStateImpl();

  boost::asio::awaitable<void> EndGame();

 private:
  LobbyManager& lobby_manager_;
  spread_logic::Game game_;
  std::string id_;
  std::vector<std::string> players_;
  std::unordered_map<std::string, std::size_t> player_to_idx_;
  std::vector<std::weak_ptr<Session>> sessions_;
  boost::asio::strand<ExecutorType> strand_;
};
