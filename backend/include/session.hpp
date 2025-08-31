#pragma once

#include <boost/asio.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/basic_stream_socket.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

class LobbyManager;     // fwd
class GameCoordinator;  // fwd

class Session : public std::enable_shared_from_this<Session> {
    using executor_type = boost::asio::io_context::executor_type;
    using strand_type = boost::asio::strand<boost::asio::io_context::executor_type>;
    using socket = boost::asio::basic_stream_socket<boost::asio::ip::tcp, executor_type>;
    using inner_socket = boost::asio::basic_stream_socket<boost::asio::ip::tcp, strand_type>;
    using websocket_stream = boost::beast::websocket::stream<inner_socket>;

   public:
    Session(socket socket, LobbyManager& lobby_manager);
    void start();
    const std::string& player_id() const {
        return player_id_;
    }

    websocket_stream& ws() {
        return ws_;
    }

   private:
    boost::asio::awaitable<void> run();
    void send_json(nlohmann::json msg);
    boost::asio::awaitable<void> route_message(nlohmann::json msg);

    // Handlers
    boost::asio::awaitable<void> handle_ping(const nlohmann::json& msg);
    boost::asio::awaitable<void> handle_list_lobbies(const nlohmann::json& msg);
    boost::asio::awaitable<void> handle_create_lobby(const nlohmann::json& msg);
    boost::asio::awaitable<void> handle_join_lobby(const nlohmann::json& msg);
    boost::asio::awaitable<void> handle_leave_lobby(const nlohmann::json& msg);
    // boost::asio::awaitable<void> handle_start_game(const nlohmann::json& msg);

   private:
    LobbyManager& lobby_manager_;
    std::weak_ptr<GameCoordinator> game_coordinator_;
    std::string player_id_;
    boost::beast::flat_buffer buffer_;
    boost::asio::strand<executor_type> strand_;
    websocket_stream ws_;
};
