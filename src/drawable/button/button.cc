#include "button.h"

#include <functional>

#include <Vector2.hpp>
#include <raylib.h>

#include "../drawable.h"

namespace fow {

Button::Button(RVector2 position, std::function<void()> action_lmb, std::function<void()> action_rmb)
  : Drawable(position), action_lmb_(action_lmb), action_rmb_(action_rmb) {}

bool Button::CheckMouse(RVector2 mouse_position, bool process_click) {
        bool handled = false;
        is_hovered_ = area_.CheckCollision(mouse_position);
        if (is_hovered_ && process_click) {
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                action_lmb_();
                handled = true;
            }
            if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
                action_rmb_();
                handled = true;
            }
        }
            return handled;

    }

} // namespace fow