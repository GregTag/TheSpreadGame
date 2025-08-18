#include "game.hpp"

#include <numeric>
#include <vector>

Game::Game(std::size_t player_count, std::uint8_t width, std::uint8_t height)
        : field_(player_count, width, height), alive_players_(player_count) {
    std::iota(alive_players_.begin(), alive_players_.end(), 1);
}

void Game::MakeMove(Coordinate pos) {
    if (winner_) {
        throw errors::GameAlreadyOver;
    }

    // Try placing the dot
    auto player = alive_players_[current_player_];
    if (!field_.PlaceDot(player, pos)) {
        throw errors::InvalidMove;
    }

    // Perform spreading chain reaction
    while (field_.SpreadStep() != 0) {
    }

    // Eliminate dead players
    UpdateAliveness();

    // Advance turn to next alive player
    NextTurn();
}

void Game::NextTurn() {
    if (alive_players_.empty()) return;
    auto n = alive_players_.size();
    current_player_ = (current_player_ + 1) % n;
    turn_count_++;
}

const std::vector<std::size_t>& Game::GetAlivePlayers() const {
    return alive_players_;
}

void Game::UpdateAliveness() {
    if (turn_count_ < alive_players_.size()) {
        return;  // Don't update aliveness until all players had at least one turn
    }

    auto scores = field_.GetPlayerScores();
    std::erase_if(alive_players_,
                  [&scores](std::size_t player_index) { return scores[player_index] == 0; });
}
