#include "field.hpp"

namespace spread_logic {

bool Square::IsFilled() const {
    return fullness >= capacity;
}

bool Square::IsOwned() const {
    return owner_index != 0;
}

bool Square::AddDot() {
    fullness++;
    return IsFilled();
}

// Field implementations
Field::Field(std::size_t player_count, std::uint8_t width, std::uint8_t height)
        : width_(width), height_(height), player_scores_(player_count + 1, 0) {
    Fill();
}

std::size_t Field::SpreadStep() {
    auto count = spread_queue_.size();
    for (std::uint8_t i = 0; i < count; ++i) {
        auto index = spread_queue_.front();
        spread_queue_.pop();
        auto& square = squares_[index];

        for (auto direction : Sides::Traverse) {
            auto neighbor_pos = MoveTo(square.position, direction);
            if (auto neighbor_index = GetIndex(neighbor_pos)) {
                auto& neighbor = squares_[*neighbor_index];
                ChangeOwner(neighbor, square.owner_index);
                if (neighbor.AddDot()) {
                    spread_queue_.push(*neighbor_index);
                }
            }
        }
        // Clear the square after spreading
        square.fullness -= square.capacity;
        if (square.IsFilled()) {
            spread_queue_.push(index);
        } else if (square.fullness == 0) {
            square.owner_index = 0;
        }
    }
    return count;
}

const std::vector<std::size_t>& Field::GetPlayerScores() const {
    return player_scores_;
}

const std::vector<Square>& Field::GetSquares() const {
    return squares_;
}

bool Field::PlaceDot(std::size_t player_index, Coordinate pos) {
    auto idxOpt = GetIndex(pos);
    if (!idxOpt) {
        return false;
    }
    auto idx = *idxOpt;
    auto& sq = squares_[idx];
    if (sq.IsOwned() && sq.owner_index != player_index) {
        return false;  // cannot place on enemy owned square
    }
    // claim ownership if neutral
    player_scores_[player_index]++;
    sq.owner_index = static_cast<std::uint8_t>(player_index);
    if (sq.AddDot()) {
        spread_queue_.push(idx);
    }

    return true;
}

void Field::Fill() {
    squares_.reserve(width_ * height_);
    for (std::int8_t y = 0; y < static_cast<std::int8_t>(height_); ++y) {
        for (std::int8_t x = 0; x < static_cast<std::int8_t>(width_); ++x) {
            auto pos = Coordinate{x, y};
            auto config = CalcConfiguration(pos);
            squares_.emplace_back(
                    pos, config,
                    static_cast<std::uint8_t>(__builtin_popcount(static_cast<unsigned>(config))));
        }
    }
}

std::uint8_t Field::CalcConfiguration(Coordinate pos) const {
    std::uint8_t config = 0;
    if (pos.y > 0) {
        config |= Sides::TOP;
    }
    if (pos.x < static_cast<std::int8_t>(width_ - 1)) {
        config |= Sides::RIGHT;
    }
    if (pos.y < static_cast<std::int8_t>(height_ - 1)) {
        config |= Sides::BOTTOM;
    }
    if (pos.x > 0) {
        config |= Sides::LEFT;
    }
    return config;
}

std::uint8_t Field::ToIndex(Coordinate pos) const {
    return static_cast<std::uint8_t>(pos.y * width_ + pos.x);
}

Coordinate Field::ToCoordinate(std::uint8_t index) const {
    return Coordinate{static_cast<std::int8_t>(index % width_),
                      static_cast<std::int8_t>(index / width_)};
}

Coordinate Field::MoveTo(Coordinate pos, std::uint8_t direction) const {
    if (direction & Sides::TOP) {
        pos.y--;
    }
    if (direction & Sides::RIGHT) {
        pos.x++;
    }
    if (direction & Sides::BOTTOM) {
        pos.y++;
    }
    if (direction & Sides::LEFT) {
        pos.x--;
    }
    return pos;
}

std::optional<std::uint8_t> Field::GetIndex(Coordinate pos) const {
    if (pos.x >= 0 && pos.x < static_cast<std::int8_t>(width_) && pos.y >= 0 &&
        pos.y < static_cast<std::int8_t>(height_)) {
        return ToIndex(pos);
    }
    return std::nullopt;
}

void Field::ChangeOwner(Square& square, std::uint8_t new_owner) {
    if (square.owner_index != 0) {
        player_scores_[square.owner_index] -= square.fullness;
    }
    player_scores_[new_owner] += square.fullness;
    square.owner_index = new_owner;
}

}  // namespace spread_logic
