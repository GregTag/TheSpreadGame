#include "models.hpp"

namespace models {
void to_json(nlohmann::json& j, const LobbyOptions& options) {
    j = nlohmann::json{{"name", options.name},
                       {"max_players", options.max_players},
                       {"width", options.width},
                       {"height", options.height}};
}

void to_json(nlohmann::json& j, const Lobby& lobby) {
    j = nlohmann::json{{"id", lobby.id},
                       {"host_player_id", lobby.host_player_id},
                       {"players", lobby.players},
                       {"options", lobby.options}};
}

void from_json(const nlohmann::json& j, Lobby& lobby) {
    j.at("id").get_to(lobby.id);
    j.at("host_player_id").get_to(lobby.host_player_id);
    j.at("players").get_to(lobby.players);
    j.at("options").get_to(lobby.options);
}

void from_json(const nlohmann::json& j, LobbyOptions& options) {
    j.at("name").get_to(options.name);
    j.at("max_players").get_to(options.max_players);
    j.at("width").get_to(options.width);
    j.at("height").get_to(options.height);
}

}  // namespace models
