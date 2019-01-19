#pragma once

#include "switch/sui.h"

#define GAME_BUTTON_WIDTH       228
#define GAME_BUTTON_HEIGHT      228

class UiGameButton : public SUIButton {
public:
    UiGameButton(std::string name);
    ~UiGameButton();

    void renderContent() override;

    SDL_Texture*& image();

private:
    SDL_Texture *image_;
};