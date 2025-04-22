#pragma once 

#include <Text.hpp>
#include <Vector2.hpp>

#include "drawable.h"

namespace fow {
	class Text : public Drawable {
	public:
		Text(RVector2 position, const RText& rtext, bool centered = true);
		void ChangeText(std::string&& text);

		void Draw() const override;
		void Scale(RVector2 scale) override;
	private:
		void Configurate();

		bool centered_;

		RText rtext_;
	};
}