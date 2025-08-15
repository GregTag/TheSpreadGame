#pragma once

#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <unordered_map>

#include "models.hpp"

// Forward declaration
class Session;

class LobbyManager {
    using execute_type = boost::asio::io_context::executor_type;

   public:
    LobbyManager(boost::asio::io_context& ioc) : strand_(ioc.get_executor()) {}
    ~LobbyManager() = default;

    // Connection lifecycle
    boost::asio::awaitable<std::string> connect(std::shared_ptr<Session> session);
    boost::asio::awaitable<void> disconnect(const std::string& player_id);

    // Lobby management
    boost::asio::awaitable<std::string> create_lobby(const std::string& player_id,
                                                     models::LobbyOptions options);
    boost::asio::awaitable<void> join_lobby(const std::string& lobby_id,
                                            const std::string& player_id);
    boost::asio::awaitable<void> leave_lobby(const std::string& player_id);
    boost::asio::awaitable<nlohmann::json> list_lobbies() const;

    // Broadcast helper
    void send_to_all(const nlohmann::json msg);
    void send_to_lobby(const std::string& lobby_id, const nlohmann::json msg);

    // Game start
    // nlohmann::json start_game(const std::string& lobby_id, const std::string& player_id);

   private:
    boost::asio::awaitable<std::string> connect_impl(std::shared_ptr<Session> session);
    boost::asio::awaitable<void> disconnect_impl(const std::string& player_id);

    boost::asio::awaitable<std::string> create_lobby_impl(const std::string& player_id,
                                                          models::LobbyOptions options);
    boost::asio::awaitable<void> join_lobby_impl(const std::string& lobby_id,
                                                 const std::string& player_id);
    boost::asio::awaitable<void> leave_lobby_impl(const std::string& player_id);
    boost::asio::awaitable<nlohmann::json> list_lobbies_impl() const;

    static boost::asio::awaitable<void> send_to_impl(
            nlohmann::json msg, std::vector<std::shared_ptr<Session>> sessions);

   private:
    boost::asio::strand<execute_type> strand_;
    // player_id -> weak session
    std::unordered_map<std::string, std::weak_ptr<Session>> sessions_;
    // lobby_id -> lobby
    std::unordered_map<std::string, models::Lobby> lobbies_;
    // player_id -> lobby_id (membership)
    std::unordered_map<std::string, std::string> membership_;
    int counter_ = 1;
    int player_counter_ = 1;
};
