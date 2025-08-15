#include "lobby_manager.hpp"

#include <spdlog/spdlog.h>

#include <boost/asio/experimental/cancellation_condition.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <sstream>

#include "errors.hpp"
#include "session.hpp"

boost::asio::awaitable<std::string> LobbyManager::connect(std::shared_ptr<Session> session) {
    return boost::asio::co_spawn(strand_, connect_impl(std::move(session)),
                                 boost::asio::use_awaitable);
}

boost::asio::awaitable<std::string> LobbyManager::connect_impl(std::shared_ptr<Session> session) {
    std::ostringstream oss;
    oss << "p" << player_counter_++;
    std::string pid = oss.str();
    sessions_[pid] = session;
    spdlog::info("Registered session for {}", pid);
    co_return pid;
}

boost::asio::awaitable<void> LobbyManager::disconnect(const std::string& player_id) {
    return boost::asio::co_spawn(strand_, disconnect_impl(player_id), boost::asio::use_awaitable);
}

boost::asio::awaitable<void> LobbyManager::disconnect_impl(const std::string& player_id) {
    sessions_.erase(player_id);
    // If in a lobby, remove the player and broadcast update
    spdlog::info("Disconnect {}", player_id);
    co_await leave_lobby_impl(player_id);
}

boost::asio::awaitable<std::string> LobbyManager::create_lobby(const std::string& player_id,
                                                               models::LobbyOptions options) {
    return boost::asio::co_spawn(strand_, create_lobby_impl(player_id, options),
                                 boost::asio::use_awaitable);
}

boost::asio::awaitable<std::string> LobbyManager::create_lobby_impl(const std::string& player_id,
                                                                    models::LobbyOptions options) {
    if (membership_.find(player_id) != membership_.end()) {
        spdlog::warn("{} already in a lobby", player_id);
        throw errors::PlayerAlreadyInLobby;
    }

    std::ostringstream oss;
    oss << "l" << counter_++;
    std::string lobby_id = oss.str();
    lobbies_[lobby_id] = models::Lobby{lobby_id, player_id, {player_id}, std::move(options)};
    membership_[player_id] = lobby_id;
    spdlog::info("Created lobby {} by {}", lobby_id, player_id);
    send_to_all({{"type", "lobby_created"}, {"lobby", lobbies_[lobby_id]}});
    co_return lobby_id;
}

boost::asio::awaitable<void> LobbyManager::join_lobby(const std::string& lobby_id,
                                                      const std::string& player_id) {
    return boost::asio::co_spawn(strand_, join_lobby_impl(lobby_id, player_id),
                                 boost::asio::use_awaitable);
}

boost::asio::awaitable<void> LobbyManager::join_lobby_impl(const std::string& lobby_id,
                                                           const std::string& player_id) {
    auto it = lobbies_.find(lobby_id);
    if (it == lobbies_.end()) {
        spdlog::warn("lobby {} not found for join by {}", lobby_id, player_id);
        throw errors::LobbyNotFound;
    }
    if (membership_.find(player_id) != membership_.end()) {
        spdlog::warn("{} already in a lobby on join {}", player_id, lobby_id);
        throw errors::PlayerAlreadyInLobby;
    }

    models::Lobby& lobby = it->second;
    lobby.players.push_back(player_id);
    membership_[player_id] = lobby_id;
    send_to_all({{"type", "lobby_update"}, {"lobby", lobby}});
    co_return;
}

boost::asio::awaitable<void> LobbyManager::leave_lobby(const std::string& player_id) {
    return boost::asio::co_spawn(strand_, leave_lobby_impl(player_id), boost::asio::use_awaitable);
}

boost::asio::awaitable<void> LobbyManager::leave_lobby_impl(const std::string& player_id) {
    auto it = membership_.find(player_id);
    if (it == membership_.end()) {
        spdlog::warn("{} tried to leave but not in lobby", player_id);
        throw errors::PlayerNotInLobby;
    }

    auto lobby_id = it->second;
    membership_.erase(it);

    auto lit = lobbies_.find(lobby_id);
    if (lit == lobbies_.end()) {
        spdlog::warn("lobby {} vanished before leave of {}", lobby_id, player_id);
        throw errors::LobbyNotFound;
    }

    models::Lobby& l = lit->second;
    auto pos = std::ranges::find(l.players, player_id);
    l.players.erase(pos);

    if (l.players.empty()) {
        spdlog::info("Removed last player {}; deleting lobby {}", player_id, lobby_id);
        lobbies_.erase(lit);
        send_to_all({{"type", "lobby_gone"}, {"lobby_id", lobby_id}});
        co_return;
    }
    if (l.host_player_id == player_id) {
        // reassign host if possible
        l.host_player_id = l.players.front();
    }
    send_to_all({{"type", "lobby_update"}, {"lobby", l}});
}

boost::asio::awaitable<nlohmann::json> LobbyManager::list_lobbies() const {
    return boost::asio::co_spawn(strand_, list_lobbies_impl(), boost::asio::use_awaitable);
}

boost::asio::awaitable<nlohmann::json> LobbyManager::list_lobbies_impl() const {
    nlohmann::json result = nlohmann::json::array();
    for (const auto& [_, lobby] : lobbies_) {
        result.push_back(lobby);
    }
    co_return result;
}

void LobbyManager::send_to_all(nlohmann::json msg) {
    std::vector<std::shared_ptr<Session>> sessions;
    sessions.reserve(sessions_.size());
    for (const auto& [id, session] : sessions_) {
        if (auto s = session.lock()) {
            sessions.emplace_back(s);
        }
    }
    boost::asio::co_spawn(strand_.get_inner_executor(),
                          send_to_impl(std::move(msg), std::move(sessions)), boost::asio::detached);
}

void LobbyManager::send_to_lobby(const std::string& lobby_id, nlohmann::json msg) {
    auto it = lobbies_.find(lobby_id);
    if (it == lobbies_.end()) {
        return;
    }
    std::vector<std::shared_ptr<Session>> sessions;
    sessions.reserve(it->second.players.size());
    for (auto const& p : it->second.players) {
        if (auto s = sessions_[p].lock()) {
            sessions.emplace_back(s);
        }
    }
    boost::asio::co_spawn(strand_.get_inner_executor(),
                          send_to_impl(std::move(msg), std::move(sessions)), boost::asio::detached);
}

boost::asio::awaitable<void> LobbyManager::send_to_impl(
        nlohmann::json msg, std::vector<std::shared_ptr<Session>> sessions) {
    auto data = msg.dump();
    auto buf = boost::asio::buffer(data);
    std::vector<boost::asio::awaitable<std::size_t>> tasks;
    tasks.reserve(sessions.size());
    for (auto const& s : sessions) {
        tasks.emplace_back(s->ws().async_write(buf, boost::asio::use_awaitable));
    }

    for (auto& task : tasks) {
        co_await std::move(task);
    }
}
