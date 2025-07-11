#include "screen.h"

#include <memory>

#include <Vector2.hpp>
#include <raylib.h>

#include "../../drawable/button/button.h"
#include "../../drawable/drawable.h"

namespace fow {

void Screen::Clear() {
  drawables_.clear();
  buttons_.clear();
  drawables_static_.clear();
  buttons_static_.clear();
}

void Screen::Draw() const {
  camera_->BeginMode();
  for (auto& drawable : drawables_) {
    drawable->Draw();
  }
  camera_->EndMode();
  for (auto& drawable_static : drawables_static_) {
    drawable_static->Draw();
  }
}

void Screen::CheckButtons() {
  RVector2 mouse_position = GetMousePosition();
  bool consumed = false;

  for (auto it = buttons_static_.rbegin(); it != buttons_static_.rend(); ++it) {
      consumed = (*it)->CheckMouse(mouse_position) || consumed;
  }

  RVector2 mouse_position_world = camera_->GetScreenToWorld(mouse_position);
  for (auto it = buttons_.rbegin(); it != buttons_.rend(); ++it) {
      consumed = (*it)->CheckMouse(mouse_position_world, !consumed) || consumed;
  }
}

void Screen::ScalePositions(float window_width, float window_height, bool inverse) {
  RVector2 scale = { window_width / basic_width_, window_height / basic_height_ };
  if (inverse) {
    scale = RVector2(1.f, 1.f) / scale;
  }
  for (auto& drawable : drawables_) {
    drawable->Scale(scale);
  }
  for (auto& drawable_static : drawables_static_) {
    drawable_static->Scale(scale);
  }
}

void Screen::PlaceDrawable(std::shared_ptr<Drawable> drawable, bool is_static) {
  auto button = std::dynamic_pointer_cast<Button>(drawable);
  if (is_static) {
    drawables_static_.push_back(drawable);
    if (button) {
      buttons_static_.push_back(button);
    }
  } else {
    drawables_.push_back(drawable);
    if (button) {
      buttons_.push_back(button);
    }
  }
}

} // namespace fow

