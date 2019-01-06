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
    this->clip.x = 0;
    this->clip.y = 0;
    this->clip.w = this->width;
    this->clip.h = this->height;

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

void SUI::drawBottomToolbar(std::initializer_list<std::tuple<std::string, SUIToolbarAction>> items) {
    int base_height = measureTextAscent(font_normal);
    int offset_x = this->width - SUI_MARGIN_SIDE - SUI_MARGIN_TOOLBAR_SIDE;
    int offset_y = this->height - SUI_MARGIN_BOTTOM + (SUI_MARGIN_BOTTOM - base_height) / 2;

    for (auto item : items) {
        SDL_Texture *icon_texture;
        int icon_width, icon_height;

        std::string text = std::get<0>(item);
        SUIToolbarAction action = std::get<1>(item);

        switch (action)
        {
        case SUIToolbarActionA:
            icon_texture = button_a_texture;
            icon_width = button_a_width;
            icon_height = button_a_height;
            break;

        case SUIToolbarActionB:
            icon_texture = button_b_texture;
            icon_width = button_b_width;
            icon_height = button_b_height;
            break;

        default:
            continue;
        }

        // Measure the size of this particular label
        int text_width;
        measureText(font_normal, text, &text_width, NULL);

        // Draw the text and icon
        drawText(font_normal, text, offset_x - text_width, offset_y, SUI_COLOR_DARK, false, -1);
        drawTexture(icon_texture,
                    offset_x - text_width - SUI_MARGIN_BETWEEN_TOOLBAR_ICON_TEXT - icon_width,
                    height - SUI_MARGIN_BOTTOM + (SUI_MARGIN_BOTTOM - icon_height) / 2,
                    icon_width,
                    icon_height);

        offset_x = offset_x - text_width - SUI_MARGIN_BETWEEN_TOOLBAR_ICON_TEXT - icon_width - SUI_MARGIN_BETWEEN_TOOLBAR_BUTTONS;
    }

    // Draw the bottom separator
    hlineColor(this->renderer, SUI_MARGIN_SIDE, this->width - SUI_MARGIN_SIDE, this->height - SUI_MARGIN_BOTTOM, SUI_COLOR_DARK);
}

void SUI::drawTopHeader(char *text) {
    // Draw the top separator
    hlineColor(this->renderer, SUI_MARGIN_SIDE, this->width - SUI_MARGIN_SIDE, SUI_MARGIN_TOP, SUI_COLOR_DARK);

    // Draw the text
    int text_width, text_height = measureTextAscent(this->font_heading);
    measureText(this->font_heading, text, &text_width, NULL);
    drawText(this->font_heading,
             text,
             SUI_MARGIN_SIDE + SUI_MARGIN_TOOLBAR_SIDE,
             (SUI_MARGIN_TOP - text_height) / 2 + 10,
             SUI_COLOR_DARK,
             false,
             -1);
}

void SUI::drawTexture(SDL_Texture *texture, int x, int y, int w, int h) {
    SUIRect dst;
    dst.x = x;
    dst.y = y;
    dst.w = w;
    dst.h = h;
    SDL_RenderCopy(this->renderer, texture, NULL, &dst);
}

void SUI::drawTexture(SDL_Texture *texture, SUIRect *bounds) {
    SDL_RenderCopy(this->renderer, texture, NULL, bounds);
}

void SUI::drawTextureClipped(SDL_Texture *texture, int x, int y, int w, int h, SUIRect *clip) {
    SUIRect clipped_destination = clipBounds(x, y, w, h, clip);
    SUIRect clipped_source;
    clipped_source.x = clipped_destination.x - x;
    clipped_source.y = clipped_destination.y - y;
    clipped_source.w = clipped_destination.w;
    clipped_source.h = clipped_destination.h;

    SDL_RenderCopy(
        this->renderer,
        texture,
        &clipped_source,
        &clipped_destination
    );
}

void SUI::drawTextureClipped(SDL_Texture *texture, SUIRect *bounds, SUIRect *clip) {
    SUIRect clipped_destination = clipBounds(bounds, clip);
    SUIRect clipped_source;
    clipped_source.x = clipped_destination.x - bounds->x;
    clipped_source.y = clipped_destination.y - bounds->y;
    clipped_source.w = clipped_destination.w;
    clipped_source.h = clipped_destination.h;

    SDL_RenderCopy(
        this->renderer,
        texture,
        &clipped_source,
        &clipped_destination
    );
}

void SUI::drawBox(int x, int y, int width, int height, uint32_t color) {
    boxColor(this->renderer, x, y, x + width, y + height, color);
}

void SUI::drawBox(SUIRect *bounds, uint32_t color) {
    drawBox(bounds->x, bounds->y, bounds->w, bounds->h, color);
}

void SUI::drawBoxClipped(int x, int y, int width, int height, SUIRect *clip, uint32_t color) {
    SUIRect in = clipBounds(x, y, width, height, clip);

    // Only draw the box if both dimensions are positive
    if (in.w > 0 && in.h > 0) {
        boxColor(this->renderer, in.x, in.y, in.x + in.w, in.y + in.h, color);
    }
}

void SUI::drawBoxClipped(SUIRect *bounds, SUIRect *clip, uint32_t color) {
    drawBoxClipped(bounds->x, bounds->y, bounds->w, bounds->h, clip, color);
}

void SUI::drawRectangle(int x, int y, int width, int height, uint32_t color) {
    drawRectangleClipped(x, y, width, height, &this->clip, color);
}

void SUI::drawRectangle(SUIRect *bounds, uint32_t color) {
    drawRectangleClipped(bounds, &this->clip, color);
}

void SUI::drawRectangleClipped(int x, int y, int width, int height, SUIRect *clip, uint32_t color) {
    SUIRect in = clipBounds(x, y, width, height, clip);

    // Only draw the rectangle if both dimensions are positive
    if (in.w > 0 && in.h > 0) {
        // Top edge
        if (y == in.y) {
            hlineColor(this->renderer, in.x, in.x + in.w, y, color);
        }

        // Bottom edge
        if ((y + height) == (in.y + in.h)) {
            hlineColor(this->renderer, in.x, in.x + in.w, in.y + in.h, color);
        }

        // Left edge
        if (x == in.x) {
            vlineColor(this->renderer, x, in.y, in.y + in.h, color);
        }

        // Right edge
        if ((x + width) == (in.x + in.w)) {
            vlineColor(this->renderer, in.x + in.w, in.y, in.y + in.h, color);
        }
    }
}

void SUI::drawRectangleClipped(SUIRect *bounds, SUIRect *clip, uint32_t color) {
    drawRectangleClipped(bounds->x, bounds->y, bounds->w, bounds->h, clip, color);
}

int SUI::measureTextAscent(TTF_Font *font) {
    return TTF_FontAscent(font);
}

void SUI::measureText(TTF_Font *font, const char *text, int *width, int *height) {
    TTF_SizeUTF8(font, text, width, height);
}

void SUI::measureText(TTF_Font *font, std::string text, int *width, int *height) {
    TTF_SizeUTF8(font, text.c_str(), width, height);
}

void SUI::drawText(TTF_Font *font, const char *text, int x, int y, uint32_t color, bool align_center, int truncate_width) {
    drawTextClipped(font, text, x, y, &this->clip, color, align_center, truncate_width);
}

void SUI::drawText(TTF_Font *font, std::string text, int x, int y, uint32_t color, bool align_center, int truncate_width) {
    drawTextClipped(font, text.c_str(), x, y, &this->clip, color, align_center, truncate_width);
}

void SUI::drawTextClipped(TTF_Font *font, const char *text, int x, int y, SUIRect *clip, uint32_t color, bool align_center, int truncate_width) {
    // Measure the text size
    int text_width, text_height;
    measureText(font, text, &text_width, &text_height);

    // Render the text into a surface
    SDL_Color fg;
    fg.r = color & 0xFF;
    fg.g = (color >> 8) & 0xFF;
    fg.b = (color >> 16) & 0xFF;
    fg.a = (color >> 24) & 0xFF;

    SDL_Surface *text_surface = TTF_RenderUTF8_Blended(font, text, fg);

    // Handle truncation
    if (truncate_width >= 0 && truncate_width < text_width)
    {
        text_width = truncate_width;

        if (SDL_MUSTLOCK(text_surface))
        {
            SDL_LockSurface(text_surface);
        }

        fprintf(stderr, "[GUI, text] Surface format: %s\n", SDL_GetPixelFormatName(text_surface->format->format));
        
        uint32_t *pixels = (uint32_t *)text_surface->pixels;
        int pitch = text_surface->pitch;

        for (size_t y = 0; y < text_height; y++)
        {
            for (size_t x = SUI_TRUNCATE_FADE_WIDTH; x > 0; x--)
            {
                size_t pos = y * (pitch / sizeof(uint32_t)) + (text_width - x);
                uint32_t origColor = pixels[pos];
                uint32_t blankColor = origColor & 0x00ffffff;
                pixels[pos] = interpolate(blankColor, origColor, (float)x / SUI_TRUNCATE_FADE_WIDTH);
            }
        }

        if (SDL_MUSTLOCK(text_surface))
        {
            SDL_UnlockSurface(text_surface);
        }
    }

    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(this->renderer, text_surface);
    SDL_FreeSurface(text_surface);

    // Render the surface at a specific location
    SUIRect destination_rect;

    if (align_center)
    {
        destination_rect.x = x - text_width / 2;
        destination_rect.y = y - text_height / 2;
        destination_rect.w = text_width;
        destination_rect.h = text_height;
    }
    else
    {
        destination_rect.x = x;
        destination_rect.y = y;
        destination_rect.w = text_width;
        destination_rect.h = text_height;
    }

    drawTextureClipped(text_texture, &destination_rect, clip);
    SDL_DestroyTexture(text_texture);
}

void SUI::drawTextClipped(TTF_Font *font, std::string text, int x, int y, SUIRect *clip, uint32_t color, bool align_center, int truncate_width) {
    drawTextClipped(font, text.c_str(), x, y, clip, color, align_center, truncate_width);
}

SUIRect SUI::clipBounds(int x, int y, int width, int height, SUIRect *clip) {
    int x1 = x,
        y1 = y,
        x2 = x + width,
        y2 = y + height;

    if (x1 < clip->x) {
        x1 = clip->x;
    }

    if (x2 > (clip->x + clip->w)) {
        x2 = clip->x + clip->w;
    }

    if (y1 < clip->y) {
        y1 = clip->y;
    }

    if (y2 > (clip->y + clip->h)) {
        y2 = clip->y + clip->h;
    }

    SUIRect intersect;
    intersect.x = x1;
    intersect.y = y1;
    intersect.w = x2 - x1;
    intersect.h = y2 - y1;
    return intersect;
}

SUIRect SUI::clipBounds(SUIRect *bounds, SUIRect *clip) {
    return clipBounds(bounds->x, bounds->y, bounds->w, bounds->h, clip);
}

uint32_t SUI::interpolate(uint32_t a, uint32_t b, double t) {
    uint8_t
        ar = (a & 0xff),
        br = (b & 0xff),
        ag = ((a >> 8) & 0xff), bg = ((b >> 8) & 0xff),
        ab = ((a >> 16) & 0xff), bb = ((b >> 16) & 0xff),
        aa = ((a >> 24) & 0xff), ba = ((b >> 24) & 0xff);

    return RGBA8(
        (uint8_t)(ar + t * (br - ar)),
        (uint8_t)(ag + t * (bg - ag)),
        (uint8_t)(ab + t * (bb - ab)),
        (uint8_t)(aa + t * (ba - aa))
    );
}