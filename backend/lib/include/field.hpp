#pragma once

#include <cstdint>
#include <optional>
#include <queue>
#include <vector>

namespace spread_logic {

struct Sides {
    enum : std::uint8_t { NONE = 0, TOP = 1, RIGHT = 2, BOTTOM = 4, LEFT = 8 };

    constexpr static std::uint8_t Traverse[] = {TOP, RIGHT, BOTTOM, LEFT};
};

struct Coordinate {
    std::int8_t x;
    std::int8_t y;
};

class Square {
   public:
    bool IsFilled() const;

    bool IsOwned() const;

    bool AddDot();

    Square() = default;
    Square(Coordinate p, std::uint8_t conf, std::uint8_t cap)
            : position(p), configuration(conf), capacity(cap) {}

    Coordinate position;
    std::uint8_t configuration;
    std::uint8_t capacity;
    std::uint8_t fullness{0};
    std::uint8_t owner_index{0};
};

class Field {
   public:
    Field(std::size_t player_count, std::uint8_t width, std::uint8_t height);

    std::size_t SpreadStep();

    const std::vector<std::size_t>& GetPlayerScores() const;

    const std::vector<Square>& GetSquares() const;

    // Place a dot for the given player at the position if rules allow (unowned or already owned by
    // that player). Returns true if the dot was placed, false if the move is invalid or out of
    // bounds.
    bool PlaceDot(std::size_t player_index, Coordinate pos);

    // Dimensions
    std::uint8_t GetWidth() const {
        return width_;
    }
    std::uint8_t GetHeight() const {
        return height_;
    }

   private:
    void Fill();

    std::uint8_t CalcConfiguration(Coordinate pos) const;

    std::uint8_t ToIndex(Coordinate pos) const;

    Coordinate ToCoordinate(std::uint8_t index) const;

    Coordinate MoveTo(Coordinate pos, std::uint8_t direction) const;

    std::optional<std::uint8_t> GetIndex(Coordinate pos) const;

    void ChangeOwner(Square& square, std::uint8_t new_owner);

    std::uint8_t width_;
    std::uint8_t height_;
    std::vector<std::size_t> player_scores_;
    std::vector<Square> squares_;
    std::queue<std::uint8_t> spread_queue_;
};

}  // namespace spread_logic
