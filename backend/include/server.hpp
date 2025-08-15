#pragma once

#include <boost/asio.hpp>
#include <boost/asio/basic_socket_acceptor.hpp>
#include <boost/asio/io_context.hpp>

#include "lobby_manager.hpp"

class Server {
    using executor_type = boost::asio::io_context::executor_type;
    using acceptor_type = boost::asio::basic_socket_acceptor<boost::asio::ip::tcp, executor_type>;

   public:
    Server(boost::asio::io_context& ioc, unsigned short port);
    void start();

   private:
    boost::asio::awaitable<void> do_accept();

    boost::asio::io_context& ioc_;
    acceptor_type acceptor_;

    LobbyManager lobby_manager_;
};
