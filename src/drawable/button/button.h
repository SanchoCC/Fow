#pragma once

#include <functional>

#include <Rectangle.hpp>
#include <Vector2.hpp>

#include "../drawable.h"

namespace fow {

class Button : public Drawable {
  public:
  Button(RVector2 position, std::function<void()> action_lmb, std::function<void()> action_rmb);
  virtual ~Button() {};
  //void CheckMouse(RVector2 mouse_position);
  bool CheckMouse(RVector2 mouse_position, bool process_click = true);

  RRectangle GetArea() const { return area_; }
  void SetIsSelected(bool is_selected) { is_selected_ = is_selected; }
  bool GetIsHovered() const { return is_hovered_; }
  protected:
  bool is_selected_ = false;
  bool is_hovered_ = false;
  RRectangle area_;
  std::function<void()> action_lmb_;
  std::function<void()> action_rmb_;
};

} // namespace fow