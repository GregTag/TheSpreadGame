#pragma once

#include <list>
#include <stdexcept>
#include <vector>

#include "field.hpp"

namespace spread_logic {

namespace errors {
const std::logic_error kInvalidMove{
    "Invalid move: out of bounds or not allowed"};
const std::logic_error kPlayerNotAlive{"Invalid move: player is not alive"};
const std::logic_error kGameAlreadyOver{"Game is already over"};
}  // namespace errors

struct Move {
  std::size_t player_index;
  std::size_t cell_id;
};

class Game {
 public:
  Game(std::size_t player_count, std::uint8_t width, std::uint8_t height);

  // Active player index (1-based to match owner_index in Field)
  std::size_t GetCurrentPlayer() const {
    return *current_player_;
  }

  std::size_t GetCurrentTurn() const {
    return turn_count_;
  }

  const std::vector<Move>& GetMoveHistory() const {
    return move_history_;
  }

  const Field& GetField() const {
    return field_;
  }

  // Make a move for the active player at position.
  void MakeMove(std::size_t cell_id);

  // Advance to next alive player
  void NextTurn();

  // Eliminate a player, noop if not alive
  void EliminatePlayer(std::size_t player_idx);

  // Return indices of alive players (1-based)
  const std::list<std::size_t>& GetAlivePlayers() const;

 private:
  void UpdateAliveness();

  Field field_;
  std::vector<Move> move_history_;
  std::list<std::size_t> alive_players_;
  std::list<std::size_t>::iterator current_player_;
  std::size_t turn_count_{0};
};

#ifdef SPREAD_LOGIC_ENABLE_JSON
// NOLINTBEGIN(readability-identifier-naming)
void to_json(nlohmann::json& j, const Move& move);

void from_json(const nlohmann::json& j, Move& move);
// NOLINTEND(readability-identifier-naming)
#endif

}  // namespace spread_logic
