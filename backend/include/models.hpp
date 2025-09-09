#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace models {

struct LobbyOptions {
  std::string name;
  int max_players = 4;
  int width = 8;
  int height = 8;
};

enum class LobbyStatus { Open, InProgress, Finished };

struct Lobby {
  std::string id;
  std::string host_player_id;
  std::vector<std::string> players;  // connected players in lobby
  LobbyOptions options;
  LobbyStatus status = LobbyStatus::Open;
};

// NOLINTBEGIN(readability-identifier-naming)
void to_json(nlohmann::json& j, const LobbyOptions& options);
void to_json(nlohmann::json& j, const Lobby& lobby);

void from_json(const nlohmann::json& j, LobbyOptions& options);
void from_json(const nlohmann::json& j, Lobby& lobby);
// NOLINTEND(readability-identifier-naming)

}  // namespace models
