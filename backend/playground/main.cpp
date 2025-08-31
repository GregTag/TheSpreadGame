#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include "game.hpp"

static void print_board(const spread_logic::Field& field) {
    const auto& squares = field.GetSquares();
    auto w = field.GetWidth();
    auto h = field.GetHeight();
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const auto& sq = squares[y * w + x];
            char owner = '.';
            if (sq.owner_index != 0) owner = 'A' + static_cast<char>(sq.owner_index - 1);
            std::cout << owner;
            std::cout << static_cast<int>(sq.fullness);
            if (x + 1 < w) std::cout << ' ';
        }
        std::cout << "\n";
    }
}

int main() {
    std::cout << "Spread Playground\n";

    int w = 4, h = 4, p = 2;
    std::cout << "Enter width height players (default 4 4 2): ";
    {
        std::string line;
        std::getline(std::cin, line);
        if (!line.empty()) {
            std::istringstream iss(line);
            iss >> w >> h >> p;
            if (w <= 0) w = 4;
            if (h <= 0) h = 4;
            if (p < 2) p = 2;
        }
    }

    spread_logic::Game game(static_cast<std::size_t>(p), static_cast<std::uint8_t>(w),
                            static_cast<std::uint8_t>(h));

    while (true) {
        std::cout << "\nCurrent board (owner+fullness):\n";
        print_board(game.GetField());
        if (auto alive = game.GetAlivePlayers(); alive.size() == 1) {
            std::cout << "Winner: Player " << static_cast<char>('A' + alive.front() - 1) << "\n";
            break;
        }
        auto active = game.GetCurrentPlayer();
        std::cout << "Player " << static_cast<char>('A' + active - 1) << " move (x y): ";
        int x = -1, y = -1;
        if (!(std::cin >> x >> y)) {
            std::cout << "Input ended.\n";
            break;
        }
        auto pos = spread_logic::Coordinate{static_cast<int8_t>(x), static_cast<int8_t>(y)};
        // consume trailing newline
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        try {
            game.MakeMove(pos);
        } catch (const std::logic_error& e) {
            std::cout << e.what() << "\n";
            continue;
        }

        std::cout << "Scores: ";
        for (const auto& score : game.GetField().GetPlayerScores()) {
            std::cout << score << " ";
        }
        std::cout << "\n";
    }

    return 0;
}
