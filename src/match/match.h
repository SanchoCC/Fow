#pragma once

#include <memory>
#include <vector>

#include <Camera2D.hpp>

#include "map/map.h"
#include "player.h"
#include "targets/units/unit.h"

namespace fow {

class Match {
public:
  void InitMap(int rows, int columns);
  void InitPlayers(float basic_width, float basic_height, RCamera2D camera, int number = 2);
  void EndTurn();
  void DeleteDeadUnits();

  const std::unique_ptr<Map>& GetMap() const { return map_; }

  Player& GetCurrentPlayer() { return players_.at(current_player_index_); }
  std::vector<Player> GetOtherPlayers() const;
  const UnitManager& GetUnitManager() { return unit_manager_; }
private:
  void NextPlayer();
  void PlaceUnits1();
  void PlaceUnits2();

  UnitManager unit_manager_;

  std::unique_ptr<Map> map_;
  std::vector<Player> players_;

  int current_player_index_ = 0;
};

} // namespace fow