#include "texture_button.h"

namespace fow {
    TextureStates::TextureStates(std::shared_ptr<RTexture> basic, std::shared_ptr<RTexture> hovered, std::shared_ptr<RTexture> fow) {
        this->basic = basic;
        this->hovered = hovered;
        this->fow = fow;
    }

    TextureButton::TextureButton(RVector2 position, RVector2 size, std::function<void()> action, TextureStates texture)
        : Button(position, action), texture_(texture) {
        
        area_ = { position, size };
        texture.basic->width = size.GetX();
        texture.basic->height = size.GetY();
        texture.hovered->width = size.GetX();
        texture.hovered->height = size.GetY();
    }

    void TextureButton::Draw() const {
        if (is_hovered_ && texture_.hovered != nullptr) {
            texture_.hovered->Draw(position_);
        } else {
            texture_.basic->Draw(position_);
        }
    }

    void TextureButton::Scale(RVector2 scale) {
        position_ *= scale;
        RVector2 area_size = area_.GetSize() * scale;
        area_ = { position_, area_size };
    }
}