#include "screen_controller.h"

#include "../../menu/menu_screen.h"
#include "../../input/input.h"
#include "../../match/match_screen.h"

namespace fow {

	ScreenController::ScreenController(ScreenType start_screen_type, std::unique_ptr<RWindow> window) : window_(std::move(window)) {
		window_width_ = window_->GetWidth();
		window_height_= window_->GetHeight();
		
		ChangeScreen(start_screen_type, CreateCamera());
	}

	void ScreenController::ProcessScreen() {
		
		while (!window_->ShouldClose() && !current_screen_->ShouldClose() ) {

			if (draw_fps_) {
				DrawFPS(0, 0);
			}
								
			current_screen_->Update();
			current_screen_->CheckButtons();	

			window_->BeginDrawing();

			window_->ClearBackground(RColor{});

			current_screen_->Draw();

			window_->EndDrawing();

			if (current_screen_->ShouldFinish()) {
				ScreenType new_screen_type = current_screen_->Finish();
				ChangeScreen(new_screen_type, CreateCamera());
			}
		}
		window_->Close();
	}

	std::shared_ptr<RCamera2D> ScreenController::CreateCamera() {
		
		RVector2 camera_offset = { window_width_ / 2.f, window_height_ / 2.f };
		RVector2 camera_target = { 0, 0 };
		return std::make_shared<RCamera2D>(camera_offset, camera_target);
	}

	void ScreenController::ChangeScreen(ScreenType screen_type, std::shared_ptr<RCamera2D> camera) {
		current_screen_ = CreateScreen(screen_type, camera);
		current_screen_->Init();
		current_screen_->ScalePositions(window_width_, window_height_);
	}

    std::shared_ptr<Screen> ScreenController::CreateScreen(ScreenType screen_type, std::shared_ptr<RCamera2D> camera) {
		switch (screen_type) {
			case ScreenType::kMenu: {
				return std::make_shared<MenuScreen>(camera);
				break;
			}
			case ScreenType::kMatch: {
				return std::make_shared<MatchScreen>(camera);
				break;
			}
		}
    }
}