#include "sui.h"

#include "button_a_png.h"
#include "button_b_png.h"

SUI::SUI() 
    : input({})
{
    Result rc;

    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0) {
        fprintf(stderr, "[GUI] Could not initialize SDL: %s\n", SDL_GetError());
        return;
    }

    int flags = IMG_INIT_PNG;
    if (IMG_Init(flags) != flags) {
        fprintf(stderr, "[GUI] Could not initialize SDL image library: %s\n", IMG_GetError());
        return;
    }

    if (TTF_Init() < 0) {
        fprintf(stderr, "[GUI] Could not initialize SDL font library: %s\n", TTF_GetError());
        return;
    }

    if (SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &this->window, &this->renderer) < 0) {
        fprintf(stderr, "[GUI] Could not create SDL window and renderer: %s\n", SDL_GetError());
        return;
    }

    SDL_GetWindowSize(this->window, &this->width, &this->height);

    // Load the fonts
    rc = plGetSharedFontByType(&font_data, PlSharedFontType_Standard);
    if (R_FAILED(rc)) {
        fprintf(stderr, "[GUI] Could not load Switch shared font\n");
        return;
    }

    font_small = TTF_OpenFontRW(SDL_RWFromMem(font_data.address, font_data.size), 1, 18);
    font_normal = TTF_OpenFontRW(SDL_RWFromMem(font_data.address, font_data.size), 1, 22);
    font_heading = TTF_OpenFontRW(SDL_RWFromMem(font_data.address, font_data.size), 1, 28);
    font_massive = TTF_OpenFontRW(SDL_RWFromMem(font_data.address, font_data.size), 1, 64);

    if (!font_small || !font_normal || !font_heading || !font_massive) {
        fprintf(stderr, "[GUI] Could not load font into SDL: %s\n", TTF_GetError());
        return;
    }

    // Load the toolbar textures
    button_a_texture = loadPNG(button_a_png, button_a_png_size);
    if (!button_a_texture) {
        fprintf(stderr, "[GUI] Could not load button A image: %s\n", SDL_GetError());
        return;
    }
    SDL_QueryTexture(button_a_texture, NULL, NULL, &button_a_width, &button_a_height);

    button_b_texture = loadPNG(button_b_png, button_b_png_size);
    if (!button_b_texture) {
        fprintf(stderr, "[GUI] Could not load button B image: %s\n", SDL_GetError());
        return;
    }
    SDL_QueryTexture(button_b_texture, NULL, NULL, &button_b_width, &button_b_height);
}

SUI::~SUI() {
    if (button_a_texture) { SDL_DestroyTexture(button_a_texture); }
    if (button_b_texture) { SDL_DestroyTexture(button_b_texture); }

    if (font_small) { TTF_CloseFont(font_small); }
    if (font_normal) { TTF_CloseFont(font_normal); }
    if (font_heading) { TTF_CloseFont(font_heading); }
    if (font_massive) { TTF_CloseFont(font_massive); }

    if (renderer) { SDL_DestroyRenderer(renderer); }
    if (window) { SDL_DestroyWindow(window); }

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

SUIInput *SUI::inputPoll(HidControllerID id) {
    hidScanInput();

    // Read the buttons
    this->input.buttons.down = hidKeysDown(id);
    this->input.buttons.held = hidKeysHeld(id);
    this->input.buttons.up = hidKeysUp(id);

    // Read both joysticks
    hidJoystickRead(&this->input.joysticks.left, id, JOYSTICK_LEFT);
    hidJoystickRead(&this->input.joysticks.right, id, JOYSTICK_RIGHT);

    // Read any touch positions
    if (hidTouchCount() > 0) {
        this->input.touch.touched = true;
        hidTouchRead(&this->input.touch.position, 0);
    }
    else {
        this->input.touch.touched = false;
    }

    return &input;
}

bool SUI::inputTest(SUIInput *input) {
    return (
        input->buttons.down ||
        input->buttons.held ||
        input->buttons.up ||
        input->joysticks.left.dx ||
        input->joysticks.left.dy ||
        input->joysticks.right.dx ||
        input->joysticks.right.dy ||
        input->touch.touched
    );
}

SDL_Texture *SUI::loadPNG(const void *data, size_t size) {
    return loadPNGScaled(data, size, -1, -1);
}

SDL_Texture *SUI::loadPNGScaled(const void *data, size_t size, int width, int height) {
    SDL_RWops *rwops;
    SDL_Surface *surface;
    SDL_Texture *texture;

    rwops = SDL_RWFromConstMem(data, size);
    if (!rwops) {
        fprintf(stderr, "[GUI] Could not create SDL RW object: %s\n", SDL_GetError());
        return NULL;
    }

    surface = IMG_LoadPNG_RW(rwops);
    if (!surface) {
        fprintf(stderr, "[GUI] Could not load PNG data into surface: %s\n", IMG_GetError());
        return NULL;
    }

    // Resize the surface if requested
    if (width != -1 && height != -1) {
        double sx = (double)width / surface->w;
        double sy = (double)height / surface->h;

        SDL_Surface *scaled = zoomSurface(surface, sx, sy, SMOOTHING_ON);
        SDL_FreeSurface(surface);
        surface = scaled;

        if (!scaled)
        {
            fprintf(stderr, "[GUI] Could not resize PNG of size (%d,%d) to size (%d,%d)\n", surface->w, surface->h, width, height);
            return NULL;
        }
    }

    texture = SDL_CreateTextureFromSurface(this->renderer, surface);
    if (!texture) {
        fprintf(stderr, "[GUI] Could not create PNG texture from surface: %s\n", SDL_GetError());
        return NULL;
    }
    SDL_FreeSurface(surface);

    return texture;
}