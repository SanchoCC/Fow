#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Camera2D.hpp"

#include "../drawable/button/button.h"
#include "../structs/vector2i.h"
#include "map/map.h"
#include "targets/units/unit.h"

namespace fow {

class Player {
public:
  void InitCamera(RCamera2D camera) { camera_ = std::make_shared<RCamera2D>(camera); }
  void InitMaps(const std::unique_ptr<Map>& map, float basic_width, float basic_height);

  void Update(const std::unique_ptr<Map>& map, std::vector<Player>&& other_players);

  void StartTurn();

  void AddUnit(Vector2I position, UnitType unit_type, const UnitManager& unit_manager);
  void SetSelectedUnit(std::shared_ptr<Unit> unit, const std::unique_ptr<Map>& map = nullptr);
  const std::shared_ptr<Unit>& GetSelectedUnit() const { return selected_unit_; }
  Vector2I GetSelectedTilePosition() const { return selected_tile_position_; }
  RRectangle GetTileArea(Vector2I position) const;
  void ClearSelectedTile();
  void ClearActionTile();

  std::shared_ptr<RCamera2D> GetCamera() { return camera_; }
  std::vector<std::shared_ptr<Unit>>& GetUnits();
  const std::vector<std::vector<std::shared_ptr<Button>>>& GetRenderMap() const { return render_map_; }
  const std::vector<std::vector<float>>& GetProbabilitiesMap() const;
  void InvertShowPrevMap() { if (turn_ != 0) show_prev_map_ = !show_prev_map_; }
  bool GetShowPrevMap() const { return show_prev_map_; }
  const std::unordered_set<Vector2I>& GetPossibleMoveTiles() const { return possible_move_tiles_; }
  const std::unordered_map<Vector2I, float>& GetMovementCosts() const { return movement_costs_; }
  const std::unordered_set<Vector2I>& GetPossibleReconTiles() const { return possible_recon_tiles_; }
  const std::unordered_set<Vector2I>& GetPossibleAttackTiles() const { return possible_attack_tiles_; }
  const std::unordered_map<Vector2I, double>& GetPossibleAttackedTiles(Vector2I attacked_tile) const;
  UnitAction GetCurrentAction() const { return current_action_; }
  // TODO: Separate methods 
  void SwapAction() { 
    if (current_action_ == UnitAction::kMove) current_action_ = UnitAction::kRecon;
    else if (current_action_ == UnitAction::kRecon) current_action_ = UnitAction::kAttack;
    else current_action_ = UnitAction::kMove;
  }

  void DoUnitAction(const std::unique_ptr<Map>& map, std::vector<Player> other_players);

  Vector2I GetHitedTile() const { return hited_tile_; }
  bool GetShowedHitedTile() const { return showed_hited_tile_; }
  void SetShowedHitedTile(bool showed_hited_tile) { showed_hited_tile_ = showed_hited_tile; }
private:
  void UpdateRenderMap();

  void UpdateProbabilitiesMap(const std::unique_ptr<Map>& map, std::vector<Player>&& other_players);
  std::vector<std::shared_ptr<Unit>> GetEnemyUnits(std::vector<Player>&& other_players) const;
  std::unordered_set<Vector2I> GetScoutedTiles() const;
  std::unordered_map<std::shared_ptr<Unit>, std::unordered_set<Vector2I>> GetPossibleTiles(
      const std::unordered_map<std::shared_ptr<Unit>, std::unordered_set<Vector2I>>& unit_related_tiles,
      const std::unordered_map<Vector2I, std::unordered_set<Vector2I>>& neighbors,
      const std::unordered_set<Vector2I>& scouted_tiles) const;
  void FillProbabilitiesMap(const std::unordered_map<Vector2I, std::unordered_set<Vector2I>>& neighbors,
      std::unordered_map<std::shared_ptr<Unit>, std::unordered_set<Vector2I>>& possible_tiles,
      const std::vector<std::shared_ptr<Unit>>& enemy_units);

  void MoveSelectedUnit(const std::unique_ptr<Map>& map, std::vector<Player>&& other_players);
  void ReconTile(const std::unique_ptr<Map>& map);
  void AttackTile(const std::unique_ptr<Map>& map, std::vector<Player>&& other_players);

  void SetSelectedTilePosition(Vector2I position);
  void SetActionTilePosition(Vector2I position);
  void ResetUnitsMovementPoints();

  void UpdatePossibleTiles(const std::unique_ptr<Map>& map);
  std::unordered_set<Vector2I> RangeCircle(Vector2I origin, int range, int min_range, const std::unique_ptr<Map>& map);
  void ClearPossibleTiles();

  std::shared_ptr<Unit> selected_unit_ = nullptr;
  Vector2I action_tile_position_ = { -1, -1 };
  Vector2I selected_tile_position_ = { -2, -2 };

  std::shared_ptr<RCamera2D> camera_;
  std::vector<std::shared_ptr<Unit>> units_;
  std::vector<std::shared_ptr<Unit>> prev_units_;

  std::vector<std::vector<std::shared_ptr<Button>>> render_map_;
  std::unordered_set<Vector2I> was_unit_tiles_;
  std::unordered_set<Vector2I> recon_tiles_;
  std::vector<std::vector<float>> probabilities_map_;
  std::vector<std::vector<float>> prev_map_;

  std::unordered_set<Vector2I> possible_move_tiles_;
  std::unordered_map<Vector2I, float> movement_costs_;
  std::unordered_set<Vector2I> possible_recon_tiles_;
  std::unordered_set<Vector2I> possible_attack_tiles_;

  bool show_prev_map_ = false;
  bool should_update_probabilities_map_ = true;

  UnitAction current_action_ = UnitAction::kMove;
  mutable Vector2I attacked_tile_ = { -1, -1 };
  mutable std::unordered_map<Vector2I, double> possible_attacked_tiles_;
  Vector2I hited_tile_ = { -1, -1 };
  bool showed_hited_tile_ = true;

  int turn_ = -1;
};

} // namespace fow