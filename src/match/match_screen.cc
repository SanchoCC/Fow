#include "match_screen.h"

#include <Image.hpp>
#include <Texture.hpp>
#include "map/terrain.h"

#include "../drawable/button/texture_button.h"

namespace fow {
    void MatchScreen::Init() {

        match_ = std::make_unique<Match>();
        match_->InitMap(16, 30);
        match_->InitPlayers(basic_width_, basic_height_, *camera_.get());
    }

    void MatchScreen::Update() {
        drawables_.clear();
        buttons_.clear();      

        auto& player = match_->GetCurrentPlayer();
        camera_ = player.GetCamera();
        PlaceButtonsFromRenderMap(player.GetRenderMap());

        CheckInputs();              
    }

    ScreenType MatchScreen::Finish() {
        return ScreenType::kMenu;
    }

    void MatchScreen::CheckInputs() {
        if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
            input.DragScreen(camera_.get());
        }
        float mouse_wheel = GetMouseWheelMove();
        if (std::abs(mouse_wheel) > 0.0f) {
            input.Zoom(camera_.get(), mouse_wheel * 0.05f, 0.5f, 2.f);
        }

        if (IsKeyPressed(KEY_ENTER)) {
            match_->EndTurn();
        }
    }

    void MatchScreen::PlaceButtonsFromRenderMap(std::vector<std::vector<std::shared_ptr<TextureButton>>> buttons) {
        for (auto& row : buttons) {
            for (auto& button : row) {
                PlaceButton(button);
            }
        }
    }
}