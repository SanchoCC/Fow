#include "player.h"

#include <memory>
#include <vector>

#include <Camera2D.hpp>

#include "map/map.h"
#include "match.h"
#include "targets/units/unit.h"
#include "../utils/astar.h"

namespace fow {

void Match::InitMap(int rows, int columns) {
  // hills mountains marsh forest water urban plains  
  // Hill-mount map
  // TerrainDistribution distribution { 0.15f, 0.1f, 0.f, 0.1f, 0.05f, 0.05f, 0.5f };
  // Forest-swamp map
   TerrainDistribution distribution{ 0.f, 0.f, 0.15f, 0.2f, 0.1f, 0.05f, 0.5f };

  int k = 25;
  // k clustering iterations per tile
  // k = 0: random map
  // 1 <= k <= 20: low clusterization
  // 21 <= k <= 40: normal clusterization
  // 41 <= k <= 60: high clusterization
  // k >= 61: very high clusterization
  map_ = std::make_unique<Map>(rows, columns, distribution, k);

  map_->GenerateRoads(20, 0.25);
}

void Match::InitPlayers(float basic_width, float basic_height, RCamera2D camera, int number) {
  players_.resize(number);
  for (auto& player : players_) {
    player.InitCamera(camera);
    player.InitMaps(map_, basic_width, basic_height);
  }
  players_[0].StartTurn();
  PlaceUnits2();
  current_player_index_ = 0;
}

void Match::EndTurn() {
  NextPlayer();
  players_[current_player_index_].StartTurn();
}

void Match::DeleteDeadUnits() {
  for (auto& player : players_) {
    auto& units = player.GetUnits();
    for (auto it = units.begin(); it != units.end();) {
      if (it->get()->GetHealthPoints() < 0.1f) {
        it = units.erase(it);
      } else {
        ++it;
      }
    }
  }
}

std::vector<Player> Match::GetOtherPlayers() const {
  std::vector<Player> other_players = players_;
  other_players.erase(other_players.begin() + current_player_index_);
  return other_players;
}

void Match::NextPlayer() {
  if (current_player_index_ >= players_.size() - 1) {
    current_player_index_ = 0;
  } else {
    ++current_player_index_;
  }
}

void Match::PlaceUnits1() {
  // Player 1
  players_[0].AddUnit({ 3, 3 }, UnitType::kAntiTank, unit_manager_);
  players_[0].AddUnit({ 15, 7 }, UnitType::kArtillery, unit_manager_);
  players_[0].AddUnit({ 3, 5 }, UnitType::kEW, unit_manager_);
  players_[0].AddUnit({ 2, 4 }, UnitType::kInfantry, unit_manager_);
  players_[0].AddUnit({ 3, 11 }, UnitType::kMechanizedInfantry, unit_manager_);
  players_[0].AddUnit({ 3, 13 }, UnitType::kTank, unit_manager_);
  players_[0].AddUnit({ 15, 8 }, UnitType::kTBM, unit_manager_);
  players_[0].AddUnit({ 3, 9 }, UnitType::kUCAV, unit_manager_);
  // Player 2
  players_[1].AddUnit({ 3, 2 }, UnitType::kInfantry, unit_manager_);
  players_[1].AddUnit({ 3, 4 }, UnitType::kInfantry, unit_manager_);
}

void Match::PlaceUnits2() {
  // Player 1
  players_[0].AddUnit({ 3, 5 }, UnitType::kArtillery, unit_manager_);
  players_[0].AddUnit({ 5, 5 }, UnitType::kArtillery, unit_manager_);
  players_[0].AddUnit({ 3, 7 }, UnitType::kArtillery, unit_manager_);
  players_[0].AddUnit({ 5, 7 }, UnitType::kArtillery, unit_manager_);
  players_[0].AddUnit({ 3, 9 }, UnitType::kArtillery, unit_manager_);
  players_[0].AddUnit({ 5, 9 }, UnitType::kArtillery, unit_manager_);
  players_[0].AddUnit({ 3, 11 }, UnitType::kArtillery, unit_manager_);
  players_[0].AddUnit({ 5, 11 }, UnitType::kArtillery, unit_manager_);

  players_[0].AddUnit({ 11, 3 }, UnitType::kInfantry, unit_manager_);
  players_[0].AddUnit({ 11, 4 }, UnitType::kInfantry, unit_manager_);
  players_[0].AddUnit({ 10, 5 }, UnitType::kInfantry, unit_manager_);
  players_[0].AddUnit({ 11, 6 }, UnitType::kInfantry, unit_manager_);
  players_[0].AddUnit({ 11, 7 }, UnitType::kInfantry, unit_manager_);
  players_[0].AddUnit({ 10, 8 }, UnitType::kInfantry, unit_manager_);
  players_[0].AddUnit({ 11, 9 }, UnitType::kInfantry, unit_manager_);
  players_[0].AddUnit({ 11, 10 }, UnitType::kInfantry, unit_manager_);
  players_[0].AddUnit({ 10, 11 }, UnitType::kInfantry, unit_manager_);
  players_[0].AddUnit({ 11, 12 }, UnitType::kInfantry, unit_manager_);
  players_[0].AddUnit({ 11, 13 }, UnitType::kInfantry, unit_manager_);

  // Player 2
  players_[1].AddUnit({ 13, 3 }, UnitType::kInfantry, unit_manager_);
  players_[1].AddUnit({ 12, 4 }, UnitType::kInfantry, unit_manager_);
  players_[1].AddUnit({ 13, 5 }, UnitType::kInfantry, unit_manager_);
  players_[1].AddUnit({ 12, 6 }, UnitType::kInfantry, unit_manager_);
  players_[1].AddUnit({ 13, 7 }, UnitType::kInfantry, unit_manager_);
  players_[1].AddUnit({ 12, 8 }, UnitType::kInfantry, unit_manager_);
  players_[1].AddUnit({ 13, 9 }, UnitType::kInfantry, unit_manager_);
  players_[1].AddUnit({ 12, 10 }, UnitType::kInfantry, unit_manager_);
  players_[1].AddUnit({ 13, 11 }, UnitType::kInfantry, unit_manager_);
  players_[1].AddUnit({ 12, 12 }, UnitType::kInfantry, unit_manager_);
  players_[1].AddUnit({ 13, 13 }, UnitType::kInfantry, unit_manager_);

  players_[1].AddUnit({ 20, 8 }, UnitType::kTBM, unit_manager_);
  players_[1].AddUnit({ 20, 12 }, UnitType::kTBM, unit_manager_);

  players_[1].AddUnit({ 14, 1 }, UnitType::kArtillery, unit_manager_);
  players_[1].AddUnit({ 14, 2 }, UnitType::kArtillery, unit_manager_);
  players_[1].AddUnit({ 14, 15 }, UnitType::kArtillery, unit_manager_);
  players_[1].AddUnit({ 14, 16 }, UnitType::kArtillery, unit_manager_);
}

} // namespace fow