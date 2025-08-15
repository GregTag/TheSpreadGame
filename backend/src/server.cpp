#include "server.hpp"

#include <spdlog/spdlog.h>

#include <boost/asio.hpp>
#include <boost/asio/detached.hpp>

#include "lobby_manager.hpp"
#include "session.hpp"

using boost::asio::ip::tcp;

Server::Server(boost::asio::io_context& ioc, unsigned short port)
        : ioc_(ioc), acceptor_(ioc, tcp::endpoint(tcp::v4(), port)), lobby_manager_(ioc) {}

void Server::start() {
    boost::asio::co_spawn(ioc_, do_accept(), boost::asio::detached);
}

boost::asio::awaitable<void> Server::do_accept() {
    while (true) {
        auto [ec, socket] =
                co_await acceptor_.async_accept(boost::asio::as_tuple(boost::asio::use_awaitable));
        if (!ec) {
            spdlog::info("Accepted connection from {}",
                         socket.remote_endpoint().address().to_string());
            std::make_shared<Session>(std::move(socket), lobby_manager_)->start();
        } else {
            spdlog::error("Accept error: {}", ec.message());
        }
    }
}
