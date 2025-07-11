#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <Color.hpp>
#include <Text.hpp>
#include <Vector2.hpp>

#include "../drawable/button/text_button.h"
#include "../drawable/rectangle.h"
#include "../drawable/text.h"
#include "../drawable/drawable.h"

namespace fow {
	class Window {
	public:
		Window(RVector2 pos, RVector2 size);
		Window(RVector2 pos, RVector2 size, const RText& text);

		std::vector<std::shared_ptr<Drawable>>& GetDrawables();

		bool ShouldClose() const {
			return should_close_;
		}

		void InitWindow(RVector2 position, RVector2 size, const std::optional<RText>& text);

		std::vector<std::shared_ptr<Drawable>> drawables_;
		std::shared_ptr<TextButton> close_button_;
		bool should_close_ = false;
	};

}