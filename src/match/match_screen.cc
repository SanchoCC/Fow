#include "match_screen.h"

#include <cstdlib>
#include <format>
#include <initializer_list>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <Color.hpp>
#include <Rectangle.hpp>
#include <Text.hpp>
#include <Vector2.hpp>
#include <raylib.h>

#include "../core/screen/screen.h"
#include "../drawable/button/text_button.h"
#include "../drawable/button/texture_button.h"
#include "../drawable/complex_drawable.h"
#include "../drawable/drawable.h"
#include "../drawable/rectangle.h"
#include "../drawable/text.h"
#include "../drawable/texture_manager.h"
#include "../structs/vector2i.h"

#include "match.h"
#include "player.h"
#include "targets/units/unit.h"

namespace fow {

void MatchScreen::Init() {
  InitMatch();
  InitSelectedUnitHud();
  InitPanelHud();
}

void MatchScreen::InitMatch() {
  match_ = std::make_unique<Match>();
  match_->InitMap(20, 32);
  match_->InitPlayers(basic_width_, basic_height_, *camera_.get());
}

void MatchScreen::InitPanelHud() {
  RVector2 origin = { basic_width_, basic_height_ };
  float shift = 130.f;

  RColor background_color = RColor::Black().Alpha(0.875f);
  RVector2 background_shift = { shift, 120.f };
  background_shift *= 2.f;
  RVector2 background_position = origin - background_shift;
  std::shared_ptr<Rectangle> background_rectangle = std::make_shared<Rectangle>(background_position, background_shift, background_color);
  ComplexDrawablePart background("BG", background_rectangle);

  float font_size = 40.f;

  RVector2 show_fow_position = origin - RVector2(shift, 200.f);
  RText show_fow_text("SHOW FOW", font_size);
  auto show_fow_button = std::make_shared<TextButton>(show_fow_position, [&]() { show_fow = !show_fow; }, show_fow_text, true);
  ComplexDrawablePart show_fow("SHOW_FOW", show_fow_button);

  RVector2 show_zeros_position = origin - RVector2(shift, 150.f);
  RText show_zeros_text("SHOW 0.0%", font_size);
  auto show_zeros_button = std::make_shared<TextButton>(show_zeros_position, [&]() { show_zero = !show_zero; }, show_zeros_text, true);
  ComplexDrawablePart show_zeros("SHOW_ZEROS", show_zeros_button);

  RVector2 prev_map_position = origin - RVector2(shift, 100.f);
  RText prev_map_text("PREV MAP", font_size);
  auto InvertMaps = [&]() {
    auto& player = match_->GetCurrentPlayer();
    player.InvertShowPrevMap();
  };
  auto prev_map_button = std::make_shared<TextButton>(prev_map_position, InvertMaps, prev_map_text, true);
  ComplexDrawablePart prev_map("PREV_MAP", prev_map_button);

  RVector2 end_turn_position = origin - RVector2(shift, 50.f);
  RText end_turn_text("END TURN", font_size);
  auto end_turn_button = std::make_shared<TextButton>(end_turn_position, [&]() { match_->EndTurn(); }, end_turn_text, true);
  ComplexDrawablePart end_turn("END_TURN", end_turn_button);

  std::initializer_list<ComplexDrawablePart> parts = { background, show_fow, show_zeros, prev_map, end_turn  };
  panel_hud_ = std::make_unique<ComplexDrawable>(parts);
}

void MatchScreen::InitSelectedUnitHud() {
  float shift = 120.f;
  RVector2 origin(basic_width_ / 2.f, basic_height_ - shift);

  RColor background_color = RColor::Black().Alpha(0.875f);
  // RVector2 background_shift(130.f, 40.f); // during swap action button
  RVector2 background_shift(130.f, 80.f);
  RVector2 background_position = origin - background_shift;
  RVector2 background_size = background_shift * 2.f;
  background_size.y += shift;
  std::shared_ptr<Rectangle> background_rectangle = std::make_shared<Rectangle>(background_position, background_size, background_color);
  ComplexDrawablePart background("BG", background_rectangle);

  std::shared_ptr<Text> unit_type_text = std::make_shared<Text>(origin, RText("", 40.f));
  ComplexDrawablePart unit_type("UNIT_TYPE", unit_type_text);

  std::shared_ptr<Text> hp_text = std::make_shared<Text>(origin + RVector2(0.f, 40.f), RText("", 25.f), true);
  ComplexDrawablePart hp("HP", hp_text);

  std::shared_ptr<Text> mp_text = std::make_shared<Text>(origin + RVector2(0.f, 80.f), RText("", 25.f), true);
  ComplexDrawablePart mp("MP", mp_text);
  
  // TODO: Separate buttons
  auto SwapAction = [this]() { match_->GetCurrentPlayer().SwapAction(); };
  std::shared_ptr<TextButton> swap_action_button = std::make_shared<TextButton>(origin - RVector2(0.f, 60.f), SwapAction, RText("SWITCH ACTION", 25.f));
  ComplexDrawablePart swap_action("SWAP_ACTION", swap_action_button);

  std::initializer_list<ComplexDrawablePart> parts = { background, unit_type, hp, mp, swap_action };
  selected_unit_hud_ = std::make_unique<ComplexDrawable>(parts);
}

void MatchScreen::Update() {
  match_->DeleteDeadUnits();
  auto& player = match_->GetCurrentPlayer();
  player.Update(match_->GetMap(), match_->GetOtherPlayers());

  camera_ = player.GetCamera();
  PlacePlayerButtons(player);
  UpdateTileInfoWindow(player);

  if (auto& unit = player.GetSelectedUnit(); unit) {
    ShowSelectedUnitHud(unit, match_->GetUnitManager());
  }

  if (player.GetShowPrevMap()) {
    auto layer = std::make_shared<Rectangle>(RVector2(0.f, 0.f), RVector2(10000.f, 10000.f), RColor::Gray().Alpha(0.2f));
    PlaceDrawable(layer, true);
  }
  if (!player.GetShowedHitedTile() && show_hit_stopwatch_ == -1.f) {
    show_hit_stopwatch_ = 0.f;   
  } else if (!player.GetShowedHitedTile() && show_hit_stopwatch_ < show_hit_time_) {
    show_hit_stopwatch_ += GetFrameTime();
    auto& buttons = player.GetRenderMap();
    auto tile = player.GetHitedTile();
    RRectangle area = buttons[tile.x][tile.y]->GetArea();
    RVector2 size = { area.GetSize() };
    RVector2 position = area.GetPosition();
    auto hit = std::make_shared<Rectangle>(position, size, RColor::Black());
    PlaceDrawable(hit);
  } else {
    show_hit_stopwatch_ = -1.f;
    player.SetShowedHitedTile(true);
  }

  auto& hud_drawables = panel_hud_->GetDrawables();
  for (auto& hud_drawable : hud_drawables) {
    PlaceDrawable(hud_drawable, true);
  }

  CheckInputs();
}

ScreenType MatchScreen::Finish() {
  return ScreenType::kMenu;
}

void MatchScreen::CheckInputs() {
  if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
    input.DragScreen(camera_.get());
  }
  if (float mouse_wheel = GetMouseWheelMove(); std::abs(mouse_wheel) > 0.0f) {
    input.Zoom(camera_.get(), mouse_wheel * 0.05f, 0.5f, 4.f);

  }
  if (IsKeyPressed(KEY_ENTER)) {
    match_->EndTurn();
  }
}

void MatchScreen::PlacePlayerButtons(Player& player) {
  PlaceRenderMap(player);
  PlaceProbabilityMap(player);
  PlaceUnits(player);
  PlacePossibleTiles(player);
}

void MatchScreen::PlaceRenderMap(Player& player) {
  auto& buttons = player.GetRenderMap();
  for (auto& row : buttons) {
    for (auto& button : row) {
      PlaceDrawable(button);
    }
  }
}

void MatchScreen::PlaceUnits(Player& player) {
  auto& buttons = player.GetRenderMap();
  auto& units = player.GetUnits();
  const auto& unit_manager = match_->GetUnitManager();
  for (auto& unit : units) {
    Vector2I unit_position = unit->GetPosition();
    RRectangle area = buttons[unit_position.x][unit_position.y]->GetArea();
    RVector2 size = { area.GetSize() };
    size.x *= 0.95f;
    RVector2 ratio = { 1.0, 1.5 };
    size /= ratio;
    RVector2 position = area.GetPosition();
    position += (area.GetSize() - size) / 2.f;
    auto lmb_action = [this, &player, &unit]() {
      if (player.GetSelectedUnit() == unit) {
        player.SetSelectedUnit(nullptr);
      } else {
        player.SetSelectedUnit(unit, match_->GetMap());
        player.ClearSelectedTile();
      }
    };
    std::shared_ptr<TextureButton> button = std::make_shared<TextureButton>(position, size, lmb_action, unit_manager.GetTexture(unit->GetType()));

    button->SetIsSelected(player.GetSelectedUnit() == unit);

    PlaceDrawable(button);
  }
}

void MatchScreen::PlaceProbabilityMap(Player& player) {
  auto& buttons = player.GetRenderMap();
  auto& probability_map = player.GetProbabilitiesMap();

  for (int i = 0; i < probability_map.size(); ++i) {
    for (int j = 0; j < probability_map[0].size(); ++j) {
      Vector2I tile_position = { i, j };
      RRectangle area = buttons[i][j]->GetArea();
      RVector2 size = { area.GetSize() };
      RVector2 position = area.GetPosition();
      float probability = probability_map[i][j];
      std::string text_string = "";
      if (probability == 0.f && !show_zero) {
        continue;
      }
      if (probability < 0.f) {
        if (show_fow) {
          std::shared_ptr<Rectangle> fow = std::make_shared<Rectangle>(position, size, RColor::Black().Alpha(0.25));
          PlaceDrawable(fow);
        }
        continue;
      }
      position += size / 2.f;     
      if (probability >= 0.f) {
        text_string = std::format("{:.1f}", probability * 100) + '%';
      }
      float r = 255.f;
      float g = 255.f;
      float b = 0.f;
      if (probability == 0.f) {
        r = 200.f;
        b = 200.f;
      } else if (probability < 0.5f) {
        r *= probability + 0.45f;
      } else {
        g *= 1 - probability;
      }
      
      RColor color(r, g, b);
      RText rtext(text_string, size.GetX() * 0.3f, color);

      std::shared_ptr<Text> text = std::make_shared<Text>(position, rtext);
      PlaceDrawable(text);
    }
  }
}

void MatchScreen::PlacePossibleTiles(Player& player) {
  if (player.GetShowPrevMap()) {
    return;
  }
  auto& buttons = player.GetRenderMap();

  UnitAction current_action = player.GetCurrentAction();

  std::unordered_set<Vector2I> tiles;
  RColor color;
  
  switch (current_action) {
    case UnitAction::kMove:
      tiles = player.GetPossibleMoveTiles();
      color = RColor::Green();    
      break;
    case UnitAction::kRecon:
      tiles = player.GetPossibleReconTiles();
      color = RColor::Yellow();
      break;
    case UnitAction::kAttack:
      tiles = player.GetPossibleAttackTiles();
      color = RColor::Red();
      break;
    case UnitAction::kReinforce:
      break;
  }

  std::vector<std::shared_ptr<Drawable>> draw_later;
  for (const auto& tile : tiles) {
    auto& button = buttons[tile.x][tile.y];
    RRectangle area = button->GetArea();
    RVector2 size = { area.GetSize() };
    RVector2 position = area.GetPosition();
    std::shared_ptr<Rectangle> rectangle = std::make_shared<Rectangle>(position, size, color.Alpha(0.35));
    PlaceDrawable(rectangle);
    if (current_action == UnitAction::kMove) {
      const std::unordered_map<Vector2I, float>& movement_costs = player.GetMovementCosts();
      position += size / 2.f;
      position.y += size.y / 3.f;
      std::string move_cost_str = std::format("{:.1f}", movement_costs.at(tile));
      RText rtext(move_cost_str, size.GetX() * 0.33f);
      std::shared_ptr<Text> move_cost = std::make_shared<Text>(position, rtext);
      PlaceDrawable(move_cost);
    } else if (current_action == UnitAction::kAttack) {
      if (button->GetIsHovered()) {
        const auto& attacked_tiles = player.GetPossibleAttackedTiles(tile);
        for (const auto& [attacked_tile, hit_chance] : attacked_tiles) {
          if (attacked_tile.x < 0 || attacked_tile.x >= buttons.size()
            || attacked_tile.y < 0 || attacked_tile.y >= buttons[0].size()) {
            continue;
          }
          auto& attacked_button = buttons[attacked_tile.x][attacked_tile.y];
          RRectangle attacked_area = attacked_button->GetArea();
          RVector2 attacked_position = attacked_area.GetPosition();
          std::shared_ptr<Rectangle> attacked = std::make_shared<Rectangle>(attacked_position, size, RColor(136, 8, 8).Alpha(0.6));
          draw_later.emplace_back(attacked);
          attacked_position += size / 2.f;
          attacked_position.y -= size.y / 3.f;
          std::string hit_chance_str = std::format("{:.0f}", hit_chance*100.0);
          RText rtext(hit_chance_str+'%', size.GetX() * 0.33f, RColor::Black());
          std::shared_ptr<Text> hit_chance = std::make_shared<Text>(attacked_position, rtext);     
          draw_later.emplace_back(hit_chance);
        }
      }
    }
  } 
  for (auto& drawable : draw_later) {
    PlaceDrawable(drawable);
  }
}

void MatchScreen::UpdateTileInfoWindow(Player& player) {
    Vector2I selected = player.GetSelectedTilePosition();

    if (selected != last_selected_tile_) {
        tile_info_window_.reset();

        if (selected.x >= 0 && selected.y >= 0) {
            const auto& map = match_->GetMap();
            const auto& tiles = map->GetTiles();
            auto terrain = tiles[selected.x][selected.y].GetTerrain();
            TerrainModifiers mods = terrain->GetModifiers();
            std::string info = map->GetTerrainManager().GetName(terrain->GetType());

            info += "\nAttack bonus: " + std::format("{:+.0f}%", mods.attack_bonus * 100);
            info += "\nDefense bonus: " + std::format("{:+.0f}%", mods.defense_bonus * 100);
            info += "\nMovement cost: " + std::format("{:.1f}", mods.movement_cost);

            RRectangle area = player.GetTileArea(selected);
            RVector2 center = area.GetPosition() + area.GetSize() / 2.f;
            RVector2 screen_center = camera_->GetWorldToScreen(center);
            RVector2 size = { 220.f, 120.f };
            RVector2 pos = screen_center + size / 12.f;
            tile_info_window_ = std::make_unique<Window>(pos, size, RText(info, 20.f));
        }

        last_selected_tile_ = selected;
    }

    if (tile_info_window_) {
        auto& drawables = tile_info_window_->GetDrawables();
        for (auto& drawable : drawables) {
            PlaceDrawable(drawable, true);
        }
        if (tile_info_window_->ShouldClose()) {
            tile_info_window_.reset();
        }
    }
}

void MatchScreen::ShowSelectedUnitHud(const std::shared_ptr<Unit>& unit, const UnitManager& unit_manager) {
  UnitType unit_type = unit->GetType();
  auto unit_modifiers = unit_manager.GetResource(unit_type);
  selected_unit_hud_->EditText("UNIT_TYPE", unit_manager.GetName(unit_type));

  std::string current_hp = std::format("{:.1f}", unit->GetHealthPoints());
  std::string max_hp = std::format("{:.1f}", unit_modifiers->start_health_points);
  selected_unit_hud_->EditText("HP", "HP: " + current_hp + " / " + max_hp);

  std::string current_mp = std::format("{:.1f}", unit->GetMovementPoints());
  std::string max_mp = std::format("{:.1f}", unit_modifiers->start_movement_points);
  selected_unit_hud_->EditText("MP", "MP: " + current_mp + " / " + max_mp);

  auto& hud_drawables = selected_unit_hud_->GetDrawables();
  for (auto& hud_drawable : hud_drawables) {
    PlaceDrawable(hud_drawable, true);
  }
}

} // namespace fow