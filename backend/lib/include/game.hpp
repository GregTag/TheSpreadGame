#pragma once

#include <optional>
#include <stdexcept>
#include <vector>

#include "field.hpp"

namespace errors {
const std::logic_error InvalidMove{"Invalid move: out of bounds or not allowed"};
const std::logic_error PlayerNotAlive{"Invalid move: player is not alive"};
const std::logic_error GameAlreadyOver{"Game is already over"};
}  // namespace errors

class Game {
   public:
    Game(std::size_t player_count, std::uint8_t width, std::uint8_t height);

    // Active player index (1-based to match owner_index in Field)
    std::size_t GetCurrentPlayer() const {
        return alive_players_[current_player_];
    }

    const Field& GetField() const {
        return field_;
    }

    // Make a move for the active player at position.
    void MakeMove(Coordinate pos);

    // Advance to next alive player
    void NextTurn();

    // Return indices of alive players (1-based)
    const std::vector<std::size_t>& GetAlivePlayers() const;

   private:
    void UpdateAliveness();

    Field field_;
    std::vector<std::size_t> alive_players_;
    std::size_t current_player_{0};  // 0..alive_players_.size()-1
    std::size_t turn_count_{0};
    std::optional<std::size_t> winner_;
};
