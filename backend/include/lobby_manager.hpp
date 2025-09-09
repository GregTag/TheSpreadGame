#pragma once

#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

#include "game_coordinator.hpp"
#include "models.hpp"

// Forward declaration
class Session;

class LobbyManager {
  using ExecutorType = boost::asio::io_context::executor_type;

 public:
  explicit LobbyManager(boost::asio::io_context& ioc)
      : strand_(ioc.get_executor()) {
  }
  ~LobbyManager() = default;

  // Connection lifecycle
  boost::asio::awaitable<std::string> Connect(std::shared_ptr<Session> session);
  boost::asio::awaitable<void> Disconnect(const std::string& player_id);

  // Lobby management
  boost::asio::awaitable<std::string> CreateLobby(const std::string& player_id,
                                                  models::LobbyOptions options);
  boost::asio::awaitable<void> JoinLobby(const std::string& lobby_id,
                                         const std::string& player_id);
  boost::asio::awaitable<void> LeaveLobby(const std::string& player_id);
  boost::asio::awaitable<nlohmann::json> ListLobbies() const;

  boost::asio::awaitable<void> StartGame(const std::string& player_id);
  boost::asio::awaitable<void> EndGame(const std::string& lobby_id);

 private:
  // Running under strand
  void SendToAll(const nlohmann::json msg);
  void SendToLobby(const std::string& lobby_id, const nlohmann::json msg);

  boost::asio::awaitable<std::string> ConnectImpl(
      std::shared_ptr<Session> session);
  boost::asio::awaitable<void> DisconnectImpl(const std::string& player_id);

  boost::asio::awaitable<std::string> CreateLobbyImpl(
      const std::string& player_id, models::LobbyOptions options);
  boost::asio::awaitable<void> JoinLobbyImpl(const std::string& lobby_id,
                                             const std::string& player_id);
  boost::asio::awaitable<void> LeaveLobbyImpl(const std::string& player_id);
  boost::asio::awaitable<nlohmann::json> ListLobbiesImpl() const;
  boost::asio::awaitable<void> StartGameImpl(const std::string& player_id);
  boost::asio::awaitable<void> UpdateStatusImpl(const std::string& lobby_id,
                                                models::LobbyStatus status);

 private:
  boost::asio::strand<ExecutorType> strand_;
  // lobby_id -> active game
  std::unordered_map<std::string, std::weak_ptr<GameCoordinator>> games_;
  // player_id -> weak session
  std::unordered_map<std::string, std::weak_ptr<Session>> sessions_;
  // lobby_id -> lobby
  std::unordered_map<std::string, models::Lobby> lobbies_;
  // player_id -> lobby_id (membership)
  std::unordered_map<std::string, std::string> membership_;
  int lobby_counter_ = 1;
  int player_counter_ = 1;
};
