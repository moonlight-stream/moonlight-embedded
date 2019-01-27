#pragma once

#include "switch/sui.h"
#include "../application-info.h"

#define GAME_BUTTON_WIDTH       228
#define GAME_BUTTON_HEIGHT      228

class UiGameButton : public SUIButton {
public:
    UiGameButton(std::string name);
    ~UiGameButton();

    void renderContent() override;
    void update(SUIInput *) override;

    inline SDL_Texture*& image() {
        return image_;
    }

    inline ApplicationInfo*& app() {
        return app_;
    }

private:
    ApplicationInfo *app_;
    SDL_Texture *image_;

    bool image_loaded_;
};