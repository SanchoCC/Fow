#include "core.h"

#include <memory>
#include <type_traits>

#include <Window.hpp>
#include <raylib.h>

#include "screen/screen.h"
#include "screen/screen_controller.h"

namespace fow {
Core& Core::Get() {
  static Core instance;
  return instance;
}

void Core::Start() {
  // Initialization 
  SetTraceLogLevel(log_level_);

  std::unique_ptr<RWindow> window = InitWindow(1600, 900, true);
  screen_controller_ = std::make_unique<ScreenController>(ScreenType::kMenu, std::move(window));
  // Main cycle
  screen_controller_->ProcessScreen();
}

std::unique_ptr<RWindow> Core::InitWindow(int width, int height, bool fullscreen) {
  std::unique_ptr<RWindow> window;
  window->Init(width, height, "Fow");
  if (fullscreen) {
    int display = GetCurrentMonitor();
    window->SetSize(GetMonitorWidth(display), GetMonitorHeight(display));
    //window->SetFullscreen(fullscreen);
  }
  window->SetExitKey(KEY_NULL);
  window->SetTargetFPS(60);

  return window;
}

} // namespace fow