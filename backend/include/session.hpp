#pragma once

#include <boost/asio.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/basic_stream_socket.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/system/detail/error_code.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

class LobbyManager;     // fwd
class GameCoordinator;  // fwd

class Session : public std::enable_shared_from_this<Session> {
  using ExecutorType = boost::asio::io_context::executor_type;
  using StrandType =
      boost::asio::strand<boost::asio::io_context::executor_type>;
  using Socket =
      boost::asio::basic_stream_socket<boost::asio::ip::tcp, ExecutorType>;
  using InnerSocket =
      boost::asio::basic_stream_socket<boost::asio::ip::tcp, ExecutorType>;
  using WebsocketStream = boost::beast::websocket::stream<InnerSocket>;
  using MessageType = std::shared_ptr<std::string>;
  using ChannelType = boost::asio::experimental::concurrent_channel<
      ExecutorType, void(boost::system::error_code, MessageType)>;

 public:
  Session(Socket socket, LobbyManager& lobby_manager);
  void Start();
  const std::string& PlayerId() const;

  void SetGame(std::shared_ptr<GameCoordinator> game);

  void Send(MessageType message);

 private:
  boost::asio::awaitable<void> RunReader();
  boost::asio::awaitable<void> RunWriter();
  void SendJson(nlohmann::json msg);
  boost::asio::awaitable<void> RouteMessage(nlohmann::json msg);

  // Handlers
  boost::asio::awaitable<void> HandlePing(const nlohmann::json& msg);
  boost::asio::awaitable<void> HandleListLobbies(const nlohmann::json& msg);
  boost::asio::awaitable<void> HandleCreateLobby(const nlohmann::json& msg);
  boost::asio::awaitable<void> HandleJoinLobby(const nlohmann::json& msg);
  boost::asio::awaitable<void> HandleLeaveLobby(const nlohmann::json& msg);
  boost::asio::awaitable<void> HandleStartGame(const nlohmann::json& msg);
  boost::asio::awaitable<void> HandleMakeMove(const nlohmann::json& msg);

 private:
  LobbyManager& lobby_manager_;
  std::atomic<std::shared_ptr<GameCoordinator>> game_coordinator_;
  std::string player_id_;
  boost::beast::flat_buffer buffer_;
  WebsocketStream ws_;
  ChannelType channel_;
};
