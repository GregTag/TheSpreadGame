#include "game.hpp"

#include <numeric>
#include <vector>

namespace spread_logic {

Game::Game(std::size_t player_count, std::uint8_t width, std::uint8_t height)
    : field_(player_count, width, height),
      alive_players_(player_count),
      current_player_(alive_players_.begin()) {
  std::iota(alive_players_.begin(), alive_players_.end(), 1);
}

void Game::MakeMove(std::size_t cell_id) {
  if (alive_players_.size() <= 1) {
    throw errors::kGameAlreadyOver;
  }

  // Try placing the dot
  if (!field_.PlaceDot(*current_player_, cell_id)) {
    throw errors::kInvalidMove;
  }

  move_history_.emplace_back(Move{*current_player_, cell_id});

  // Perform spreading chain reaction
  while (field_.SpreadStep() != 0) {
  }

  // Eliminate dead players
  UpdateAliveness();

  // Advance turn to next alive player
  NextTurn();
}

void Game::NextTurn() {
  if (alive_players_.empty()) {
    return;
  }
  turn_count_++;
  current_player_++;
  if (current_player_ == alive_players_.end()) {
    current_player_ = alive_players_.begin();
  }
}

void Game::EliminatePlayer(std::size_t player_idx) {
  field_.GetPlayerScores()[player_idx] = 0;

  if (player_idx != *current_player_) {
    std::erase(alive_players_, player_idx);
    return;
  }

  current_player_ = alive_players_.erase(current_player_);
  if (current_player_ == alive_players_.end() && !alive_players_.empty()) {
    current_player_ = alive_players_.begin();
  }
}

const std::list<std::size_t>& Game::GetAlivePlayers() const {
  return alive_players_;
}

void Game::UpdateAliveness() {
  if (turn_count_ < alive_players_.size()) {
    return;  // Don't update aliveness until all players had at least one turn
  }

  const auto& scores = field_.GetPlayerScores();
  std::erase_if(alive_players_, [&scores](std::size_t player_index) {
    return scores[player_index] == 0;
  });
}

#ifdef SPREAD_LOGIC_ENABLE_JSON
void to_json(nlohmann::json& j, const Move& move) {
  j = nlohmann::json{{"player_index", move.player_index},
                     {"cell_id", move.cell_id}};
}

void from_json(const nlohmann::json& j, Move& move) {
  j.at("player_index").get_to(move.player_index);
  j.at("cell_id").get_to(move.cell_id);
}
#endif

}  // namespace spread_logic
