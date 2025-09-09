#pragma once

#include <cstdint>
#include <optional>
#include <queue>
#include <vector>

namespace spread_logic {

struct Sides {
  enum : std::uint8_t { NONE = 0, TOP = 1, RIGHT = 2, BOTTOM = 4, LEFT = 8 };

  constexpr static std::uint8_t kTraverse[] = {TOP, RIGHT, BOTTOM, LEFT};
};

struct Coordinate {
  std::int32_t x;
  std::int32_t y;
};

class Cell {
 public:
  bool IsFilled() const;

  bool IsOwned() const;

  bool AddDot();

  Cell() = default;
  Cell(Coordinate p, std::uint8_t conf, std::uint8_t cap)
      : position(p),
        configuration(conf),
        capacity(cap) {
  }

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

  std::vector<std::size_t>& GetPlayerScores();
  const std::vector<std::size_t>& GetPlayerScores() const;

  const std::vector<Cell>& GetCells() const;

  // Place a dot for the given player at the position if rules allow (unowned or
  // already owned by that player). Returns true if the dot was placed, false if
  // the move is invalid or out of bounds.
  bool PlaceDot(std::size_t player_index, std::size_t cell_idx);

  std::optional<std::size_t> GetIndex(Coordinate pos) const;

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

  std::size_t ToIndex(Coordinate pos) const;

  Coordinate ToCoordinate(std::size_t index) const;

  Coordinate MoveTo(Coordinate pos, std::uint8_t direction) const;

  void ChangeOwner(Cell& cell, std::uint8_t new_owner);

  std::uint8_t width_;
  std::uint8_t height_;
  std::vector<std::uint64_t> player_scores_;
  std::vector<Cell> cells_;
  std::queue<std::size_t> spread_queue_;
};

}  // namespace spread_logic

#ifdef SPREAD_LOGIC_ENABLE_JSON

#include <nlohmann/json.hpp>

namespace spread_logic {
// NOLINTBEGIN(readability-identifier-naming)
void to_json(::nlohmann::json& j, const Cell& cell);
void to_json(::nlohmann::json& j, const Field& field);

void from_json(const ::nlohmann::json& j, Cell& cell);
// NOLINTEND(readability-identifier-naming)
}  // namespace spread_logic

#endif
