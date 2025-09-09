#pragma once

#include <boost/asio.hpp>
#include <boost/asio/basic_socket_acceptor.hpp>
#include <boost/asio/io_context.hpp>

#include "lobby_manager.hpp"

class Server {
  using ExecutorType = boost::asio::io_context::executor_type;
  using AcceptorType =
      boost::asio::basic_socket_acceptor<boost::asio::ip::tcp, ExecutorType>;

 public:
  Server(boost::asio::io_context& ioc, unsigned short port);
  void Start();

 private:
  boost::asio::awaitable<void> DoAccept();

  boost::asio::io_context& ioc_;
  AcceptorType acceptor_;

  LobbyManager lobby_manager_;
};
