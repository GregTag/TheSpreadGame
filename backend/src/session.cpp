#include "session.hpp"

#include <spdlog/spdlog.h>

#include <boost/asio/bind_executor.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/compose.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/stream_base.hpp>

#include "lobby_manager.hpp"

using boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;

Session::Session(socket socket, LobbyManager& lobby_manager)
        : lobby_manager_(lobby_manager)
        , strand_(socket.get_executor())
        , ws_(strand_, tcp::v4(), socket.release()) {}

void Session::start() {
    boost::asio::co_spawn(
            ws_.get_executor(),
            [self = shared_from_this()]() -> boost::asio::awaitable<void> { co_await self->run(); },
            boost::asio::detached);
}

boost::asio::awaitable<void> Session::run() {
    ws_.set_option(websocket::stream_base::timeout::suggested(boost::beast::role_type::server));
    ws_.set_option(websocket::stream_base::decorator([](websocket::response_type& res) {
        res.set(boost::beast::http::field::server, std::string("Spread-Server"));
    }));

    boost::beast::error_code ec;
    co_await ws_.async_accept(boost::asio::redirect_error(boost::asio::use_awaitable, ec));
    if (ec) {
        spdlog::error("WebSocket accept failed: {}", ec.message());
        co_return;
    }
    player_id_ = co_await lobby_manager_.connect(shared_from_this());
    spdlog::info("Player {} connected", player_id_);
    send_json({{"type", "server_ready"},
               {"player_id", player_id_},
               {"message", "Welcome to Spread server"}});

    while (true) {
        co_await ws_.async_read(buffer_,
                                boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        if (ec) {
            spdlog::info("Player {} disconnected: {}", player_id_, ec.message());
            break;
        }
        try {
            auto data = boost::beast::buffers_to_string(buffer_.data());
            buffer_.consume(buffer_.size());
            spdlog::debug("recv [{}]: {}", player_id_, data);
            auto msg = nlohmann::json::parse(data);
            co_await route_message(std::move(msg));
        } catch (const std::exception& ex) {
            spdlog::warn("Exception while handling message for {}: {}", player_id_, ex.what());
            send_json({{"type", "error"}, {"message", ex.what()}});
        }
    }
    co_await lobby_manager_.disconnect(player_id_);
}

void Session::send_json(nlohmann::json msg) {
    ws_.async_write(boost::asio::buffer(msg.dump()), boost::asio::detached);
}

boost::asio::awaitable<void> Session::route_message(nlohmann::json msg) {
    try {
        std::string type = msg.value("type", "");
        spdlog::info("{} -> {}", player_id_, type);
        if (type == "ping") {
            co_await handle_ping(msg);
        } else if (type == "list_lobbies") {
            co_await handle_list_lobbies(msg);
        } else if (type == "create_lobby") {
            co_await handle_create_lobby(msg);
        } else if (type == "join_lobby") {
            co_await handle_join_lobby(msg);
        } else if (type == "leave_lobby") {
            co_await handle_leave_lobby(msg);
            // } else if (type == "start_game") {
            //     co_await handle_start_game(msg);
        } else {
            spdlog::warn("{} sent unknown message type", player_id_);
            send_json({{"type", "error"}, {"message", "Unknown message type"}});
        }
    } catch (const std::exception& ex) {
        spdlog::error("Route error for {}: {}", player_id_, ex.what());
        send_json({{"type", "error"}, {"message", std::string("Exception: ") + ex.what()}});
    }
}

boost::asio::awaitable<void> Session::handle_ping(const nlohmann::json& msg) {
    send_json({{"type", "pong"}});
    co_return;
}

boost::asio::awaitable<void> Session::handle_list_lobbies(const nlohmann::json& msg) {
    auto lobbies = co_await lobby_manager_.list_lobbies();
    send_json({{"type", "lobby_list"}, {"lobbies", lobbies}});
}

boost::asio::awaitable<void> Session::handle_create_lobby(const nlohmann::json& msg) {
    auto board = msg.value("board_size", std::vector<int>{8, 8});
    int w = board.size() > 0 ? board[0] : 8;
    int h = board.size() > 1 ? board[1] : 8;
    int maxp = msg.value("max_players", 4);
    std::string name = msg.at("name");
    auto lobby_id = co_await lobby_manager_.create_lobby(player_id_, {name, maxp, w, h});
    spdlog::info("{} created lobby {}", player_id_, lobby_id);
    send_json({{"type", "joined"}, {"lobby_id", lobby_id}});
}

boost::asio::awaitable<void> Session::handle_join_lobby(const nlohmann::json& msg) {
    std::string lobby_id = msg.at("lobby_id");
    co_await lobby_manager_.join_lobby(lobby_id, player_id_);
    spdlog::info("{} joined lobby {}", player_id_, lobby_id);
    send_json({{"type", "joined"}, {"lobby_id", lobby_id}});
}

boost::asio::awaitable<void> Session::handle_leave_lobby(const nlohmann::json& msg) {
    std::string lobby_id = msg.at("lobby_id");
    co_await lobby_manager_.leave_lobby(lobby_id);
    spdlog::info("{} left lobby {}", player_id_, lobby_id);
    send_json({{"type", "left"}, {"lobby_id", lobby_id}});
}

// boost::asio::awaitable<void> Session::handle_start_game(const nlohmann::json& msg) {
//     auto lobby_id = msg.value("lobby_id", std::string{});
//     auto resp = lobby_manager_.start_game(lobby_id, player_id_);
//     send_json(resp);
// }
