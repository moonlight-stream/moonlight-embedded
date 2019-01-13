#pragma once

#include "sui-common.h"
#include "sui-graphics.h"
#include "sui-element.h"
#include "sui-stage.h"
#include "sui-container.h"
#include "sui-grid-container.h"
#include "sui-button.h"
#include "sui-image.h"
#include "sui-fps-counter.h"

class SUI {
public:
    SUI();
    ~SUI();

    SUIInput *inputPoll(HidControllerID id);
    bool inputTest(SUIInput *input);

    SDL_Texture *loadPNG(const void *data, size_t size);
    SDL_Texture *loadPNGScaled(const void *data, size_t size, int width, int height);

    SDL_Window *window;
    SDL_Renderer *renderer;
    int width;
    int height;
    SUIRect bounds;

    SUIInput input;

    PlFontData font_data;
    TTF_Font *font_small;
    TTF_Font *font_normal;
    TTF_Font *font_heading;
    TTF_Font *font_massive;
    SDL_Texture *button_a_texture;
    SDL_Texture *button_b_texture;

    int button_a_width;
    int button_a_height;
    int button_b_width;
    int button_b_height;
};
