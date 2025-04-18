#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <Camera2D.hpp>
#include <Text.hpp>

#include "../../drawable/button/button.h"
#include "../../drawable/drawable.h"
#include "../../drawable/text.h"

namespace fow {

    enum class ScreenType {
        kMenu,
        kMatch
    };

    class Screen {
    public:
        Screen(std::shared_ptr<RCamera2D> camera) : camera_(camera) {}
        virtual ~Screen() {};

        virtual void Init() = 0;
        virtual void Update() = 0;     
        virtual ScreenType Finish() = 0;

        void Draw() const;
        void CheckButtons();
        void ScalePositions(float window_width, float window_height);

        bool ShouldClose() const { return should_close_; }
        bool ShouldFinish() const { return should_finish_; }
    protected:
        void AddRText(std::string&& name, RText&& rtext);

        void PlaceText(std::shared_ptr<Text> text);
        void PlaceButton(std::shared_ptr<Button> button);

        std::vector<std::shared_ptr<Drawable>> drawables_;
        std::vector<std::shared_ptr<Button>> buttons_;

        std::unordered_map<std::string, RText> rtexts_;

        std::shared_ptr<RCamera2D> camera_;

        bool should_close_ = false;
        bool should_finish_ = false;

        float basic_width_ = 1600.f;
        float basic_height_ = 900.f;
    };

}