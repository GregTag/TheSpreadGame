#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <game.hpp>
#include <memory>

#include "models.hpp"

// Forward declaration
class Session;

class GameCoordinator : std::enable_shared_from_this<GameCoordinator> {
    using executor_type = boost::asio::io_context::executor_type;

   public:
    static std::shared_ptr<GameCoordinator> FromLobby(models::Lobby);

   private:
    GameCoordinator(/*...*/);

   private:
    spread_logic::Game game_;
    std::vector<std::shared_ptr<Session>> sessions_;
    boost::asio::strand<executor_type> strand_;
};
