#pragma once 

#include <memory>

#include "../core/screen/screen.h"
#include "../drawable/complex_drawable.h"
#include "../input/input.h"
#include "match.h"
#include "player.h"
#include "targets/units/unit.h"
#include "../structs/vector2i.h"
#include "../gui/window.h"
#include "../structs/vector2i.h"
#include <Camera2D.hpp>

namespace fow {

class MatchScreen : public Screen {
public:
  MatchScreen(std::shared_ptr<RCamera2D> camera) : Screen(camera) {}

  void Init() override;
  void Update() override;
  ScreenType Finish() override;

  void CheckInputs();
private:
  void PlacePlayerButtons(Player& player);
  void PlaceRenderMap(Player& player);
  void PlaceUnits(Player& player);
  void PlaceProbabilityMap(Player& player);
  void PlacePossibleTiles(Player& player);

  void UpdateTileInfoWindow(Player& player);
  void ShowSelectedUnitHud(const std::shared_ptr<Unit>& unit, const UnitManager& unit_manager);

  void InitMatch();
  void InitPanelHud();
  void InitSelectedUnitHud();

  std::unique_ptr<ComplexDrawable> selected_unit_hud_;
  std::unique_ptr<ComplexDrawable> panel_hud_;
  std::unique_ptr<Window> tile_info_window_;
  Vector2I last_selected_tile_{ -2, -2 };

  std::unique_ptr<Match> match_;
  Input input;
  float show_hit_time_ = 0.5f;
  float show_hit_stopwatch_ = 0.0;

  bool show_prev_map_ = false;
  bool show_fow = true;
  bool show_zero = false;
};

} // namespace fow