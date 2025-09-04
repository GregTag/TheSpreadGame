#include "session.hpp"

#include <spdlog/spdlog.h>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/compose.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/stream_base.hpp>

#include "errors.hpp"
#include "game_coordinator.hpp"
#include "lobby_manager.hpp"

namespace websocket = boost::beast::websocket;

Session::Session(Socket socket, LobbyManager& lobby_manager)
    : lobby_manager_(lobby_manager),
      ws_(std::move(socket)),
      channel_(ws_.get_executor(), 1) {
}

void Session::SetGame(std::shared_ptr<GameCoordinator> game) {
  game_coordinator_.store(std::move(game));
}

void Session::Send(MessageType message) {
  channel_.async_send({}, std::move(message), boost::asio::detached);
}

void Session::Start() {
  boost::asio::co_spawn(
      ws_.get_executor(),
      [self = shared_from_this()]() -> boost::asio::awaitable<void> {
        co_await self->RunReader();
      },
      boost::asio::detached);

  boost::asio::co_spawn(
      ws_.get_executor(),
      [self = shared_from_this()]() -> boost::asio::awaitable<void> {
        co_await self->RunWriter();
      },
      boost::asio::detached);
}

boost::asio::awaitable<void> Session::RunReader() {
  ws_.set_option(websocket::stream_base::timeout::suggested(
      boost::beast::role_type::server));
  ws_.set_option(
      websocket::stream_base::decorator([](websocket::response_type& res) {
        res.set(boost::beast::http::field::server,
                std::string("Spread-Server"));
      }));

  boost::beast::error_code ec;
  co_await ws_.async_accept(
      boost::asio::redirect_error(boost::asio::use_awaitable, ec));
  if (ec) {
    spdlog::error("WebSocket accept failed: {}", ec.message());
    co_return;
  }

  player_id_ = co_await lobby_manager_.Connect(shared_from_this());
  spdlog::info("Player {} connected", player_id_);
  SendJson({{"type", "server_ready"},
            {"player_id", player_id_},
            {"message", "Welcome to Spread server"}});

  while (true) {
    co_await ws_.async_read(
        buffer_, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
    if (ec) {
      spdlog::info("Player {} disconnected, reader: {}", player_id_,
                   ec.message());
      break;
    }
    try {
      auto data = boost::beast::buffers_to_string(buffer_.data());
      buffer_.consume(buffer_.size());
      spdlog::debug("recv [{}]: {}", player_id_, data);
      auto msg = nlohmann::json::parse(data);
      co_await RouteMessage(std::move(msg));
    } catch (const std::exception& ex) {
      spdlog::warn("Exception while handling message for {}: {}", player_id_,
                   ex.what());
      SendJson({{"type", "error"}, {"message", ex.what()}});
    }
  }
  co_await lobby_manager_.Disconnect(player_id_);
}

boost::asio::awaitable<void> Session::RunWriter() {
  while (true) {
    auto message = co_await channel_.async_receive(boost::asio::use_awaitable);
    if (!message) {
      continue;
    }
    boost::beast::error_code ec;
    co_await ws_.async_write(
        boost::asio::buffer(*message),
        boost::asio::redirect_error(boost::asio::use_awaitable, ec));
    if (ec) {
      spdlog::info("Player {} disconnected, writer: {}", player_id_,
                   ec.message());
      break;
    }
  }
}

void Session::SendJson(nlohmann::json msg) {
  Send(std::make_shared<std::string>(msg.dump()));
}

boost::asio::awaitable<void> Session::RouteMessage(nlohmann::json msg) {
  try {
    std::string type = msg.value("type", "");
    spdlog::info("{} -> {}", player_id_, type);
    if (type == "ping") {
      co_await HandlePing(msg);
    } else if (type == "list_lobbies") {
      co_await HandleListLobbies(msg);
    } else if (type == "create_lobby") {
      co_await HandleCreateLobby(msg);
    } else if (type == "join_lobby") {
      co_await HandleJoinLobby(msg);
    } else if (type == "leave_lobby") {
      co_await HandleLeaveLobby(msg);
    } else if (type == "start_game") {
      co_await HandleStartGame(msg);
    } else if (type == "make_move") {
      co_await HandleMakeMove(msg);
    } else {
      spdlog::warn("{} sent unknown message type", player_id_);
      SendJson({{"type", "error"}, {"message", "Unknown message type"}});
    }
  } catch (const std::exception& ex) {
    spdlog::error("Route error for {}: {}", player_id_, ex.what());
    SendJson({{"type", "error"},
              {"message", std::string("Exception: ") + ex.what()}});
  }
}

boost::asio::awaitable<void> Session::HandlePing(const nlohmann::json& msg) {
  SendJson({{"type", "pong"}});
  co_return;
}

boost::asio::awaitable<void> Session::HandleListLobbies(
    const nlohmann::json& msg) {
  auto lobbies = co_await lobby_manager_.ListLobbies();
  SendJson({{"type", "lobby_list"}, {"lobbies", lobbies}});
}

boost::asio::awaitable<void> Session::HandleCreateLobby(
    const nlohmann::json& msg) {
  auto board = msg.value("board_size", std::vector<int>{8, 8});
  int w = board.size() > 0 ? board[0] : 8;
  int h = board.size() > 1 ? board[1] : 8;
  int maxp = msg.value("max_players", 4);
  std::string name = msg.at("name");
  auto lobby_id =
      co_await lobby_manager_.CreateLobby(player_id_, {name, maxp, w, h});
  spdlog::info("{} created lobby {}", player_id_, lobby_id);
  SendJson({{"type", "joined"}, {"lobby_id", lobby_id}});
}

boost::asio::awaitable<void> Session::HandleJoinLobby(
    const nlohmann::json& msg) {
  std::string lobby_id = msg.at("lobby_id");
  co_await lobby_manager_.JoinLobby(lobby_id, player_id_);
  spdlog::info("{} joined lobby {}", player_id_, lobby_id);
  SendJson({{"type", "joined"}, {"lobby_id", lobby_id}});
}

boost::asio::awaitable<void> Session::HandleLeaveLobby(
    const nlohmann::json& msg) {
  (void)msg;  // Unused
  co_await lobby_manager_.LeaveLobby(player_id_);
  spdlog::info("{} left their lobby", player_id_);
  SendJson({{"type", "left"}});
}

boost::asio::awaitable<void> Session::HandleStartGame(
    const nlohmann::json& msg) {
  co_await lobby_manager_.StartGame(player_id_);
  co_return;
}

boost::asio::awaitable<void> Session::HandleMakeMove(
    const nlohmann::json& msg) {
  auto game = game_coordinator_.load();
  if (!game) {
    spdlog::warn("{} tried to make a move but is not in a game", player_id_);
    throw errors::kPlayerNotInGame;
  }
  auto cell_id = msg.at("cell_id").get<std::size_t>();
  co_await game->MakeMove(player_id_, cell_id);
}
const std::string& Session::PlayerId() const {
  return player_id_;
}
