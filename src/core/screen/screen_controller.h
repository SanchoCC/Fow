#pragma once 

#include "screen.h"

#include <memory>

#include "Window.hpp"

namespace fow {

    class ScreenController {
    public:
        ScreenController(ScreenType start_screen_type, std::unique_ptr<RWindow> window);

        void ProcessScreen();
    private:
        std::shared_ptr<RCamera2D> CreateCamera();
        void ChangeScreen(ScreenType screen_type, std::shared_ptr<RCamera2D> camera);

        std::shared_ptr<Screen> CreateScreen(ScreenType screen_type, std::shared_ptr<RCamera2D> camera);

        std::shared_ptr<Screen> current_screen_;

        std::unique_ptr<RWindow> window_; 

        float window_width_;
        float window_height_;

        bool draw_fps_ = true;
    };
}