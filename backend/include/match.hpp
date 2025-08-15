#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "game.hpp"

struct MatchPlayer {
    std::string player_id;
    std::string name;
};

class Match {
   public:
    Match(std::vector<MatchPlayer> players, BoardConfig cfg)
            : players_(std::move(players)), game_(cfg) {}

    nlohmann::json game_state_json(int turn_index = 0) const {
        nlohmann::json players = nlohmann::json::array();
        for (auto const& p : players_) {
            players.push_back({{"player_id", p.player_id}, {"name", p.name}});
        }
        return nlohmann::json{{"type", "game_state"},
                              {"board", game_.to_json()},
                              {"players", players},
                              {"turn", turn_index}};
    }

   private:
    std::vector<MatchPlayer> players_;
    Game game_;
};
