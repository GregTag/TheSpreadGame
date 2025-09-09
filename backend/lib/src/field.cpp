#include "field.hpp"

namespace spread_logic {

bool Cell::IsFilled() const {
  return fullness >= capacity;
}

bool Cell::IsOwned() const {
  return owner_index != 0;
}

bool Cell::AddDot() {
  fullness++;
  return IsFilled();
}

// Field implementations
Field::Field(std::size_t player_count, std::uint8_t width, std::uint8_t height)
    : width_(width),
      height_(height),
      player_scores_(player_count + 1, 0) {
  Fill();
}

std::size_t Field::SpreadStep() {
  auto count = spread_queue_.size();
  for (std::uint8_t i = 0; i < count; ++i) {
    auto index = spread_queue_.front();
    spread_queue_.pop();
    auto& cell = cells_[index];

    for (auto direction : Sides::kTraverse) {
      auto neighbor_pos = MoveTo(cell.position, direction);
      if (auto neighbor_index = GetIndex(neighbor_pos)) {
        auto& neighbor = cells_[*neighbor_index];
        ChangeOwner(neighbor, cell.owner_index);
        if (neighbor.AddDot()) {
          spread_queue_.push(*neighbor_index);
        }
      }
    }
    // Clear the cell after spreading
    cell.fullness -= cell.capacity;
    if (cell.IsFilled()) {
      spread_queue_.push(index);
    } else if (cell.fullness == 0) {
      cell.owner_index = 0;
    }
  }
  return count;
}

std::vector<std::size_t>& Field::GetPlayerScores() {
  return player_scores_;
}

const std::vector<std::size_t>& Field::GetPlayerScores() const {
  return player_scores_;
}

const std::vector<Cell>& Field::GetCells() const {
  return cells_;
}

bool Field::PlaceDot(std::size_t player_index, std::size_t cell_idx) {
  if (cell_idx >= cells_.size()) {
    return false;
  }
  auto& cell = cells_[cell_idx];
  if (cell.IsOwned() && cell.owner_index != player_index) {
    return false;  // cannot place on enemy owned cell
  }
  // claim ownership if neutral
  player_scores_[player_index]++;
  cell.owner_index = static_cast<std::uint8_t>(player_index);
  if (cell.AddDot()) {
    spread_queue_.push(cell_idx);
  }

  return true;
}

void Field::Fill() {
  cells_.reserve(width_ * height_);
  for (std::int8_t y = 0; y < static_cast<std::int8_t>(height_); ++y) {
    for (std::int8_t x = 0; x < static_cast<std::int8_t>(width_); ++x) {
      auto pos = Coordinate{x, y};
      auto config = CalcConfiguration(pos);
      cells_.emplace_back(pos, config,
                          static_cast<std::uint8_t>(__builtin_popcount(
                              static_cast<unsigned>(config))));
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

std::size_t Field::ToIndex(Coordinate pos) const {
  return static_cast<std::size_t>(pos.y * width_ + pos.x);
}

Coordinate Field::ToCoordinate(std::size_t index) const {
  return Coordinate{static_cast<std::int8_t>(index % width_),
                    static_cast<std::int8_t>(index / width_)};
}

Coordinate Field::MoveTo(Coordinate pos, std::uint8_t direction) const {
  if ((direction & Sides::TOP) != 0) {
    pos.y--;
  }
  if ((direction & Sides::RIGHT) != 0) {
    pos.x++;
  }
  if ((direction & Sides::BOTTOM) != 0) {
    pos.y++;
  }
  if ((direction & Sides::LEFT) != 0) {
    pos.x--;
  }
  return pos;
}

std::optional<std::size_t> Field::GetIndex(Coordinate pos) const {
  if (pos.x >= 0 && pos.x < static_cast<std::int32_t>(width_) && pos.y >= 0 &&
      pos.y < static_cast<std::int32_t>(height_)) {
    return ToIndex(pos);
  }
  return std::nullopt;
}

void Field::ChangeOwner(Cell& cell, std::uint8_t new_owner) {
  if (cell.owner_index != 0) {
    player_scores_[cell.owner_index] -= cell.fullness;
  }
  player_scores_[new_owner] += cell.fullness;
  cell.owner_index = new_owner;
}

#ifdef SPREAD_LOGIC_ENABLE_JSON

void to_json(::nlohmann::json& j, const Cell& cell) {
  j = ::nlohmann::json{{"x", cell.position.x},
                       {"y", cell.position.y},
                       {"configuration", cell.configuration},
                       {"capacity", cell.capacity},
                       {"fullness", cell.fullness},
                       {"owner_index", cell.owner_index}};
}

void to_json(::nlohmann::json& j, const Field& field) {
  j = ::nlohmann::json{{"width", field.GetWidth()},
                       {"height", field.GetHeight()},
                       {"cells", field.GetCells()},
                       {"scores", field.GetPlayerScores()}};
}

void from_json(const ::nlohmann::json& j, Cell& cell) {
  j.at("x").get_to(cell.position.x);
  j.at("y").get_to(cell.position.y);
  j.at("configuration").get_to(cell.configuration);
  j.at("capacity").get_to(cell.capacity);
  j.at("fullness").get_to(cell.fullness);
  j.at("owner_index").get_to(cell.owner_index);
}

#endif

}  // namespace spread_logic
