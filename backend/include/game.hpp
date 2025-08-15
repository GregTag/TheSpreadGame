#pragma once

#include <nlohmann/json.hpp>
#include <vector>

struct BoardConfig {
    int width{8};
    int height{8};
};

class Game {
   public:
    explicit Game(BoardConfig cfg) : cfg_(cfg) {}

    nlohmann::json to_json() const {
        return nlohmann::json{{"width", cfg_.width},
                              {"height", cfg_.height},
                              {"squares", nlohmann::json::array()}};
    }

   private:
    BoardConfig cfg_;
};
