#include "player.h"

#include <algorithm>
#include <iterator>
#include <memory>
#include <random>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <Vector2.hpp>

#include "../drawable/button/texture_button.h"
#include "../structs/texture_states.h"
#include "../structs/vector2i.h"
#include "../utils/set_operations.h"
#include "map/map.h"
#include "map/terrain.h"
#include "targets/units/unit.h"

namespace fow {

void Player::InitMaps(const std::unique_ptr<Map>& map, float basic_width, float basic_height) {
  auto&& tiles = map->GetTiles();
  auto& terrain_manager = map->GetTerrainManager();

  size_t columns = tiles.size();
  size_t rows = tiles[0].size();

  float edge_space = 250.f;
  basic_width -= edge_space;
  basic_height -= edge_space;

  float step_x = basic_width / columns;
  float step_y = basic_height / rows;
  float actual_step_size = std::min(step_x, step_y); 

  float grid_total_width = columns * actual_step_size;
  float grid_total_height = rows * actual_step_size;
  RVector2 start_position(-grid_total_width / 2.f, -grid_total_height / 2.f);

  float spacing = 0.035f;
  RVector2 step(actual_step_size, actual_step_size);
  RVector2 tile_size(step - step * spacing);

  render_map_.resize(columns);
  probabilities_map_.resize(columns);
  for (int i = 0; i < columns; ++i) {
    render_map_[i].reserve(rows);
    probabilities_map_[i].resize(rows);
    for (int j = 0; j < rows; ++j) {
      RVector2 factor(static_cast<float>(i), static_cast<float>(j));
      RVector2 position = start_position + (step * factor);
      auto& tile = tiles[i][j];
      TerrainType tile_type = tiles[i][j].GetTerrain()->GetType();
      TextureState tile_textures = terrain_manager.GetTexture(tile_type);
      auto lmb_action = [this, i, j]() {
        if ((selected_tile_position_.x != i || selected_tile_position_.y != j)
          && !selected_unit_) {
          ClearSelectedTile();
          SetSelectedTilePosition({ i, j });
        } else {
          ClearSelectedTile();
        }
      };
      auto rmb_action = [this, i, j]() {
        if (!show_prev_map_) {
          SetActionTilePosition({ i, j });
        }
      };
      auto&& button = std::make_shared<TextureButton>(position, tile_size, lmb_action, tile_textures, rmb_action);
      render_map_[i].emplace_back(button);
    }
  }
}
void Player::Update(const std::unique_ptr<Map>& map, std::vector<Player>&& other_players) {
  DoUnitAction(map, other_players);
  UpdateRenderMap();
  UpdateProbabilitiesMap(map, std::move(other_players));
}

void Player::DoUnitAction(const std::unique_ptr<Map>& map, std::vector<Player> other_players) {
  if (!selected_unit_) {
    return;
  }
  switch (current_action_) {
    case UnitAction::kMove:
      MoveSelectedUnit(map, std::move(other_players));
      break;
    case UnitAction::kRecon:
      ReconTile(map);
      break;
    case UnitAction::kAttack:
      AttackTile(map, std::move(other_players));
      should_update_probabilities_map_ = true;
      break;
    case UnitAction::kReinforce:
      break;
    default:
      break;
  }
  ClearActionTile();
}

void Player::UpdateRenderMap() {
  int x = selected_tile_position_.x;
  int y = selected_tile_position_.y;
  if (x >= 0 && y >= 0) {
    render_map_[x][y]->SetIsSelected(true);
  }
}

void Player::UpdateProbabilitiesMap(const std::unique_ptr<Map>& map, std::vector<Player>&& other_players) {
  if (show_prev_map_ && turn_ != 0) {
    return;
  }
  if (!should_update_probabilities_map_) {
    return;
  }

  for (auto& row : probabilities_map_) {
    std::fill(row.begin(), row.end(), -1.f);
  }

  std::vector<std::shared_ptr<Unit>> enemy_units = GetEnemyUnits(std::move(other_players));
  std::unordered_set<Vector2I> scouted_tiles = GetScoutedTiles();

  std::unordered_map<Vector2I, std::unordered_set<Vector2I>> neighbors;
  std::unordered_map<std::shared_ptr<Unit>, std::unordered_set<Vector2I>> unit_related_tiles;

  for (auto& scouted_tile : scouted_tiles) {
    neighbors[scouted_tile] = map->GetNeighbors(scouted_tile);
    for (auto& unit : enemy_units) {
      if (neighbors.at(scouted_tile).contains(unit->GetPosition())) {
        unit_related_tiles[unit].insert(scouted_tile);
      }
    }
  }

  std::unordered_map<std::shared_ptr<Unit>, std::unordered_set<Vector2I>> possible_tiles = GetPossibleTiles(unit_related_tiles, neighbors, scouted_tiles);
  FillProbabilitiesMap(neighbors, possible_tiles, enemy_units);

  should_update_probabilities_map_ = false;
}

std::vector <std::shared_ptr<Unit>> Player::GetEnemyUnits(std::vector<Player>&& other_players) const {
  std::vector<std::shared_ptr<Unit>> enemy_units;
  for (const auto& player : other_players) {
    for (auto& unit : player.units_) {
      enemy_units.push_back(unit);
    }
  }
  return enemy_units;
}

std::unordered_set<Vector2I> Player::GetScoutedTiles() const {
  std::unordered_set<Vector2I> scouted_tiles;
  for (const auto& unit : units_) {
    scouted_tiles.insert(unit->GetPosition());
  }
  for (const auto& tile : was_unit_tiles_) {
    scouted_tiles.insert(tile);
  }
  for (const auto& tile : recon_tiles_) {
    scouted_tiles.insert(tile);
  }
  return scouted_tiles;
}

std::unordered_map<std::shared_ptr<Unit>, std::unordered_set<Vector2I>> Player::GetPossibleTiles(
    const std::unordered_map<std::shared_ptr<Unit>, std::unordered_set<Vector2I>>& unit_related_tiles,
    const std::unordered_map<Vector2I, std::unordered_set<Vector2I>>& neighbors,
    const std::unordered_set<Vector2I>& scouted_tiles) const {

  std::unordered_map<std::shared_ptr<Unit>, std::unordered_set<Vector2I>> possible_tiles;

  for (auto& unit : unit_related_tiles) {
    std::vector<std::unordered_set<Vector2I>> sets;
    for (auto& tile : unit.second) {
      sets.emplace_back(neighbors.at(tile));
    }
    possible_tiles.emplace(unit.first, SetIntersection(sets));
  }

  for (auto& unit : unit_related_tiles) {
    std::unordered_set<Vector2I> not_related_tiles = SetDifference(scouted_tiles, unit.second);
    std::unordered_set<Vector2I> not_related_tiles_neighbors;
    for (auto& not_related_tile : not_related_tiles) {
      not_related_tiles_neighbors.insert(neighbors.at(not_related_tile).cbegin(), neighbors.at(not_related_tile).cend());
    }
    std::vector<std::unordered_set<Vector2I>> exclude{ possible_tiles[unit.first], not_related_tiles_neighbors, scouted_tiles };
    possible_tiles[unit.first] = SetDifference(exclude);
  }
  return possible_tiles;
}

void Player::FillProbabilitiesMap(const std::unordered_map<Vector2I, std::unordered_set<Vector2I>>& neighbors,
    std::unordered_map<std::shared_ptr<Unit>, std::unordered_set<Vector2I>>& possible_tiles,
    const std::vector<std::shared_ptr<Unit>>& enemy_units) {
  for (auto& tile : neighbors) {
    for (auto& neighbor : tile.second) {
      probabilities_map_[neighbor.x][neighbor.y] = 0.0f;    
    }
  }
  for (auto& unit : units_) {
    auto position = unit->GetPosition();
    probabilities_map_[position.x][position.y] = 0.0f;
  }
  for (auto& tile : recon_tiles_) {
    auto is_unit = [tile](std::shared_ptr<Unit> unit) { return unit->GetPosition() == tile; };
    auto is_enemy_there = [tile, &possible_tiles](std::shared_ptr<Unit> unit) { 
      if (unit->GetPosition() == tile) {
        possible_tiles[unit].clear();
        return true;
      } else {
        return false;
      };
    };
    if (std::any_of(enemy_units.cbegin(), enemy_units.cend(), is_enemy_there)) {
      probabilities_map_[tile.x][tile.y] = 1.f;
    } else if (std::none_of(units_.begin(), units_.end(), is_unit)) {
      probabilities_map_[tile.x][tile.y] = 0.f;
    } else {
      probabilities_map_[tile.x][tile.y] = -1.f;
    }
  }
  for (auto& unit : possible_tiles) {
    for (auto& tile : unit.second) {
      probabilities_map_[tile.x][tile.y] += 1.f / unit.second.size();
    }
  }
}

void Player::StartTurn() {
  ++turn_;
  selected_unit_ = nullptr;
  ClearPossibleTiles();

  show_prev_map_ = false;
  prev_units_.clear();
  prev_units_.reserve(units_.size());
  std::transform(
      units_.begin(), units_.end(),
      std::back_inserter(prev_units_),
      [](const std::shared_ptr<Unit>& src) {
    return src ? std::make_shared<Unit>(*src) : nullptr;
  }
  );

  should_update_probabilities_map_ = true;
  prev_map_ = probabilities_map_;
  was_unit_tiles_.clear();
  recon_tiles_.clear();

  ClearActionTile();
  ClearSelectedTile();
  ResetUnitsMovementPoints();
  current_action_ = UnitAction::kMove;
}

void Player::AddUnit(Vector2I position, UnitType unit_type, const UnitManager& unit_manager) {
  units_.emplace_back(std::make_shared<Unit>(position, unit_type, unit_manager.GetResource(unit_type)));
}

void Player::SetSelectedUnit(std::shared_ptr<Unit> unit, const std::unique_ptr<Map>& map) {
  selected_unit_ = unit;
  if (unit) {
    UpdatePossibleTiles(map);
  } else {
    ClearPossibleTiles();
  }
  ClearActionTile();
}

void Player::MoveSelectedUnit(const std::unique_ptr<Map>& map, std::vector<Player>&& other_players) {
  if (!possible_move_tiles_.contains(action_tile_position_)) {
    return;
  }
  const auto& tiles = map->GetTiles();
  Vector2I unit_position = selected_unit_->GetPosition();
  was_unit_tiles_.emplace(unit_position);
  selected_unit_->SubstractMovementPoints(movement_costs_.at(action_tile_position_));
  selected_unit_->SetPosition(action_tile_position_);
  should_update_probabilities_map_ = true;
  UpdatePossibleTiles(map);
}

void Player::ReconTile(const std::unique_ptr<Map>& map) {
  if (!possible_recon_tiles_.contains(action_tile_position_)) {
    return;
  }
  
  recon_tiles_.emplace(action_tile_position_);
  should_update_probabilities_map_ = true;
  UpdatePossibleTiles(map);
}

void Player::AttackTile(const std::unique_ptr<Map>& map, std::vector<Player>&& other_players) {
  if (!possible_attack_tiles_.contains(action_tile_position_)) {
    return;
  }
  Vector2I target_tile;
  if (possible_attacked_tiles_.size() == 1) {
     target_tile = possible_attacked_tiles_.begin()->first;
     if (!probabilities_map_[target_tile.x][target_tile.y]) {
       return; // Misclick avoidance
     }
  } else {
    std::vector<Vector2I> keys;
    std::vector<double> weights;
    for (const auto& [key, hit_chance] : possible_attacked_tiles_) {
      keys.push_back(key);
      weights.push_back(hit_chance);
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::discrete_distribution<> dist(weights.begin(), weights.end());
    int random_index = dist(gen);
    target_tile = keys[random_index];
    const auto& tiles = map->GetTiles();
    if (target_tile.x < 0 || target_tile.x >= tiles.size()
      || target_tile.y < 0 || target_tile.y >= tiles[0].size()) {
      // TODO: Decrease action points
      return; // Attacked out of range
    }
  }
  // TODO: Decrease action points
  
  hited_tile_ = target_tile;
  showed_hited_tile_ = false;
  std::vector<std::shared_ptr<Unit>> enemy_units = GetEnemyUnits(std::move(other_players));
  auto unit_it = std::find_if(enemy_units.begin(), enemy_units.end(), [&target_tile](std::shared_ptr<Unit> unit) { return unit->GetPosition() == target_tile; });
  // Is miss
  if (unit_it != enemy_units.end()) {
    selected_unit_->Attack(*unit_it, map);
  }
  unit_it = std::find_if(units_.begin(), units_.end(), [&target_tile](std::shared_ptr<Unit> unit) { return unit->GetPosition() == target_tile; });
  // Is ff
  if (unit_it != units_.end()) {
    selected_unit_->Attack(*unit_it, map);
  }
}

void Player::ClearSelectedTile() {
  int x = selected_tile_position_.x;
  int y = selected_tile_position_.y;

  if (x >= 0 && y >= 0) {
    render_map_[x][y]->SetIsSelected(false);
    selected_tile_position_ = { -2, -2 };
  }
}

void Player::ClearActionTile() {
  action_tile_position_ = { -1, -1 };
}

std::vector<std::shared_ptr<Unit>>& Player::GetUnits() {
  if (show_prev_map_ && turn_ != 0) {
    return prev_units_;
  } else {
    return units_;
  }
}

const std::vector<std::vector<float>>& Player::GetProbabilitiesMap() const {
  if (show_prev_map_ && turn_ != 0) {
    return prev_map_;
  } else {
    return probabilities_map_;
  }
}

const std::unordered_map<Vector2I, double>& Player::GetPossibleAttackedTiles(Vector2I attacked_tile) const {
  if (attacked_tile == attacked_tile_) {
    return possible_attacked_tiles_;
  }
  attacked_tile_ = attacked_tile;
  possible_attacked_tiles_.clear();
  const auto& unit_modifiers = selected_unit_->GetUnitModifiers();
  int range_deviation = unit_modifiers->range_deviation;
  int direction_deviation = unit_modifiers->direction_deviation;
  if (range_deviation == 0 && direction_deviation == 0) {
    possible_attacked_tiles_[attacked_tile_] = 1.0;
    return possible_attacked_tiles_;
  }
  Vector2I closest_direction = (attacked_tile_ - selected_unit_->GetPosition()).GetClosestDirection();  
  if (closest_direction.x != 0 && closest_direction.y != 0) { // Direction is diagonal
    int n = range_deviation * 2 + 1;
    const auto& hit_chances = unit_modifiers->diag_hit_chances;
    for (int i = 0; i < n; ++i) {
      auto tile = attacked_tile + Vector2I(i * closest_direction.x -(n-1)/2 * closest_direction.x, i * closest_direction.y - (n - 1) / 2 * closest_direction.y);
      // No better solution has been found yet than hardcoding each case, since the diagonals are of different lengths
      // and that would require a complex iterative algorithm.
      // For now, when the direction deviation is only 0 or 1 (even hardcoding 2 wouldn't be too bad)
      // this seems to be the best way to solve this problem.
      if (direction_deviation == 0) {
        possible_attacked_tiles_[tile] = hit_chances[i];
      } else if (direction_deviation == 1) {
        int i_l = i;
        Vector2I tile_l = tile + Vector2I(0, closest_direction.y);
        int i_m = i + 4;
        int i_r = i + 9;
        Vector2I tile_r = tile + Vector2I(closest_direction.x, 0);
        possible_attacked_tiles_[tile] = hit_chances[i_m];
        if (i < n-1) {
          possible_attacked_tiles_[tile_l] = hit_chances[i_l];
          possible_attacked_tiles_[tile_r] = hit_chances[i_r];
        }
      }
    }    
  } else { // Direction isn't diagonal 
    int n = range_deviation * 2 + 1;
    int m = direction_deviation * 2 + 1;
    bool swap_index = false;
    if (closest_direction.y == 0) {
      std::swap(n, m);
      swap_index = true;
    }
    const auto& hit_chances = unit_modifiers->hit_chances;
    for (int i = 0; i < n; ++i) {
      for (int j = 0; j < m; ++j) {
        auto tile = attacked_tile + Vector2I(j - (m-1)/2, i - (n-1)/2);
        int index;
        swap_index ? index = i * m + j : index = j * n + i;
        possible_attacked_tiles_[tile] = hit_chances[index];
      }
    }
  }
  return possible_attacked_tiles_;
}

void Player::SetSelectedTilePosition(Vector2I position) {
  selected_tile_position_ = position;
}

void Player::SetActionTilePosition(Vector2I position) {
  if (selected_unit_) {
    action_tile_position_ = position;
  }
}

void Player::ResetUnitsMovementPoints() {
  for (auto& unit : units_) {
    unit->ResetMovementPoints();
  }
}

RRectangle Player::GetTileArea(Vector2I position) const {
    return render_map_[position.x][position.y]->GetArea();
}

void Player::UpdatePossibleTiles(const std::unique_ptr<Map>& map) {
  ClearPossibleTiles();

  const auto& tiles = map->GetTiles();
  Vector2I unit_position = selected_unit_->GetPosition();
  TerrainModifiers terrain_modifiers = tiles[unit_position.x][unit_position.y].GetTerrain()->GetModifiers();
  auto closest_neighbors = map->GetNeighbors(unit_position);

  auto IsOtherUnitsThere = [this, unit_position](Vector2I tile) {
    auto IsUnitThere = [tile](std::shared_ptr<Unit> unit) {
      return unit->GetPosition() == tile; };
    return std::any_of(units_.cbegin(), units_.cend(), IsUnitThere);
  };

  auto unit_modifiers = selected_unit_->GetUnitModifiers();
  // Move tiles
  auto NotEnoughMovementPoints = [this, &unit_modifiers, &tiles, unit_position](Vector2I tile) {
    auto tile_terrain = tiles[tile.x][tile.y].GetTerrain();
    float movement_cost = tile_terrain->GetModifiers().movement_cost;
    if (unit_modifiers->is_vehicle
      && (tile_terrain->GetType() != TerrainType::kPlains && tile_terrain->GetType() != TerrainType::kUrban)) {
      movement_cost *= 2.f;
    }
    if (unit_position.x != tile.x && unit_position.y != tile.y) {
      movement_cost *= 1.5f;
    }
    if (selected_unit_->GetMovementPoints() + 0.1f < movement_cost) {
      return true;
    } else {
      movement_costs_[tile] = movement_cost;
      return false;
    }
  };
  possible_move_tiles_ = closest_neighbors;
  std::erase_if(possible_move_tiles_, IsOtherUnitsThere);
  std::erase_if(possible_move_tiles_, NotEnoughMovementPoints);
  // Recon tiles
  int recon_range = unit_modifiers->recon_range + terrain_modifiers.range_extend;
  possible_recon_tiles_ = RangeCircle(unit_position, recon_range, 0, map);

  std::erase_if(possible_recon_tiles_, IsOtherUnitsThere);
  possible_recon_tiles_ = SetDifference(possible_recon_tiles_, recon_tiles_);
  possible_recon_tiles_ = SetDifference(possible_recon_tiles_, was_unit_tiles_);

  // Attack tiles
  int min_attack_range = unit_modifiers->min_attack_range;
  int attack_range = unit_modifiers->attack_range + terrain_modifiers.range_extend;

  possible_attack_tiles_ = RangeCircle(unit_position, attack_range, min_attack_range, map);

  std::erase_if(possible_attack_tiles_, IsOtherUnitsThere);
  possible_attack_tiles_ = SetDifference(possible_attack_tiles_, was_unit_tiles_);
}

std::unordered_set<Vector2I> Player::RangeCircle(Vector2I origin, int range, int min_range, const std::unique_ptr<Map>& map) {
  std::unordered_set<Vector2I> tiles;

  auto map_tiles = map->GetTiles();

  int max_x = map_tiles.size();
  int max_y = map_tiles[0].size();

  for (int y = -range; y <= range; ++y) {
    for (int x = -range; x <= range; ++x) {
      int distance_sq = (x * x + y * y) - 1; // -1 for smooth diagonal tiles
      if (distance_sq <= range * range && (distance_sq > min_range * min_range || !min_range)) {
        Vector2I tile = origin + Vector2I(x, y);
        if (tile.x >= 0 && tile.x < max_x && tile.y >= 0 && tile.y < max_y) {
          tiles.emplace(tile);
        }
      }
    }
  }

  return tiles;
}

void Player::ClearPossibleTiles() {
  possible_move_tiles_.clear();
  movement_costs_.clear();
  possible_recon_tiles_.clear();
  possible_attack_tiles_.clear();
  attacked_tile_ = { -1, -1 };
}

} // namespace fow