#include "sui-graphics.h"
#include "sui-element.h"
#include "sui.h"

SUIGraphics::SUIGraphics(SUIElement *element)
    : element_(element)
{
    
}

SUIToolbarActionItem SUIGraphics::makeToolbarActionItem(std::string text, SUIToolbarAction action) {
    return std::make_tuple(text, action);
}

void SUIGraphics::drawBottomToolbar(std::vector<std::tuple<std::string, SUIToolbarAction>> items) {
    int base_height = measureTextAscent(ui()->font_normal);
    int offset_x = ui()->width - SUI_MARGIN_SIDE - SUI_MARGIN_TOOLBAR_SIDE;
    int offset_y = ui()->height - SUI_MARGIN_BOTTOM + (SUI_MARGIN_BOTTOM - base_height) / 2;

    for (auto item : items) {
        SDL_Texture *icon_texture;
        int icon_width, icon_height;

        std::string text = std::get<0>(item);
        SUIToolbarAction action = std::get<1>(item);

        switch (action)
        {
        case SUIToolbarAction::A:
            icon_texture = ui()->button_a_texture;
            icon_width = ui()->button_a_width;
            icon_height = ui()->button_a_height;
            break;

        case SUIToolbarAction::B:
            icon_texture = ui()->button_b_texture;
            icon_width = ui()->button_b_width;
            icon_height = ui()->button_b_height;
            break;

        default:
            continue;
        }

        // Measure the size of this particular label
        int text_width;
        measureText(ui()->font_normal, text, &text_width, NULL);

        // Draw the text and icon
        drawText(ui()->font_normal, text, offset_x - text_width, offset_y, SUI_COLOR_DARK, false, -1);
        drawTexture(icon_texture,
                    offset_x - text_width - SUI_MARGIN_BETWEEN_TOOLBAR_ICON_TEXT - icon_width,
                    ui()->height - SUI_MARGIN_BOTTOM + (SUI_MARGIN_BOTTOM - icon_height) / 2,
                    icon_width,
                    icon_height);

        offset_x = offset_x - text_width - SUI_MARGIN_BETWEEN_TOOLBAR_ICON_TEXT - icon_width - SUI_MARGIN_BETWEEN_TOOLBAR_BUTTONS;
    }

    // Draw the bottom separator
    hlineColor(ui()->renderer, SUI_MARGIN_SIDE, ui()->width - SUI_MARGIN_SIDE, ui()->height - SUI_MARGIN_BOTTOM, SUI_COLOR_DARK);
}

void SUIGraphics::drawTopHeader(std::string text) {
    // Draw the top separator
    hlineColor(ui()->renderer, SUI_MARGIN_SIDE, ui()->width - SUI_MARGIN_SIDE, SUI_MARGIN_TOP, SUI_COLOR_DARK);

    // Draw the text
    int text_width, text_height = measureTextAscent(ui()->font_heading);
    measureText(ui()->font_heading, text.c_str(), &text_width, NULL);
    drawText(ui()->font_heading,
             text.c_str(),
             SUI_MARGIN_SIDE + SUI_MARGIN_TOOLBAR_SIDE,
             (SUI_MARGIN_TOP - text_height) / 2 + 10,
             SUI_COLOR_DARK,
             false,
             -1);
}

void SUIGraphics::drawTexture(SDL_Texture *texture, int x, int y, int w, int h) {
    drawTextureClipped(texture, {x, y, w, h}, element()->globalClip());
}

void SUIGraphics::drawTexture(SDL_Texture *texture, const SUIRect &bounds) {
    drawTextureClipped(texture, bounds, element()->globalClip());
}

void SUIGraphics::drawTextureClipped(SDL_Texture *texture, int x, int y, int w, int h, const SUIRect &clip) {
    drawTextureClipped(texture, {x, y, w, h}, clip);
}

void SUIGraphics::drawTextureClipped(SDL_Texture *texture, const SUIRect &bounds, const SUIRect &clip) {
    SUIRect offset_bounds = offsetGlobal(bounds);
    SUIRect clipped_destination = clipBounds(offset_bounds, clip);
    SUIRect clipped_source = {
        clipped_destination.x - offset_bounds.x,
        clipped_destination.y - offset_bounds.y,
        clipped_destination.w,
        clipped_destination.h
    };

    SDL_RenderCopy(
        ui()->renderer,
        texture,
        &clipped_source,
        &clipped_destination
    );
}

void SUIGraphics::drawBox(int x, int y, int width, int height, uint32_t color) {
    drawBoxClipped({x, y, width, height}, element()->globalClip(), color);
}

void SUIGraphics::drawBox(const SUIRect &bounds, uint32_t color) {
    drawBoxClipped(bounds, element()->globalClip(), color);
}

void SUIGraphics::drawBoxClipped(int x, int y, int width, int height, const SUIRect &clip, uint32_t color) {
    drawBoxClipped({x, y, width, height}, clip, color);
}

void SUIGraphics::drawBoxClipped(const SUIRect &bounds, const SUIRect &clip, uint32_t color) {
    SUIRect offset_bounds = offsetGlobal(bounds);
    SUIRect in = clipBounds(offset_bounds, clip);

    if (in.w > 0 && in.h > 0) {
        boxColor(ui()->renderer, 
                 offset_bounds.x,
                 offset_bounds.y, 
                 offset_bounds.x + offset_bounds.w, 
                 offset_bounds.y + offset_bounds.h, color);
    }
}

void SUIGraphics::drawRectangle(int x, int y, int width, int height, uint32_t color) {
    drawRectangleClipped({x, y, width, height}, element()->globalClip(), color);
}

void SUIGraphics::drawRectangle(const SUIRect &bounds, uint32_t color) {
    drawRectangleClipped(bounds, element()->globalClip(), color);
}

void SUIGraphics::drawRectangleClipped(int x, int y, int width, int height, const SUIRect &clip, uint32_t color) {
    drawRectangleClipped({x, y, width, height}, clip, color);
}

void SUIGraphics::drawRectangleClipped(const SUIRect &bounds, const SUIRect &clip, uint32_t color) {
    SUIRect offset_bounds = offsetGlobal(bounds);
    SUIRect in = clipBounds(offset_bounds, clip);

    // Only draw the rectangle if both dimensions are positive
    if (in.w > 0 && in.h > 0) {
        // Top edge
        if (offset_bounds.y == in.y) {
            hlineColor(ui()->renderer, in.x, in.x + in.w, in.y, color);
        }

        // Bottom edge
        if ((offset_bounds.y + offset_bounds.h) == (in.y + in.h)) {
            hlineColor(ui()->renderer, in.x, in.x + in.w, in.y + in.h, color);
        }

        // Left edge
        if (offset_bounds.x == in.x) {
            vlineColor(ui()->renderer, in.x, in.y, in.y + in.h, color);
        }

        // Right edge
        if ((offset_bounds.x + offset_bounds.w) == (in.x + in.w)) {
            vlineColor(ui()->renderer, in.x + in.w, in.y, in.y + in.h, color);
        }
    }}

int SUIGraphics::measureTextAscent(TTF_Font *font) {
    return TTF_FontAscent(font);
}

void SUIGraphics::measureText(TTF_Font *font, std::string text, int *width, int *height) {
    TTF_SizeUTF8(font, text.c_str(), width, height);
}

void SUIGraphics::drawText(TTF_Font *font, std::string text, int x, int y, uint32_t color, bool align_center, int truncate_width) {
    drawTextClipped(font, text, x, y, element()->globalClip(), color, align_center, truncate_width);
}

void SUIGraphics::drawText(TTF_Font *font, std::string text, const SUIRect &bounds, uint32_t color, bool align_center) {
    drawTextClipped(font, text, bounds, element()->globalClip(), color, align_center);
}

void SUIGraphics::drawTextClipped(TTF_Font *font, std::string text, int x, int y, const SUIRect &clip, uint32_t color, bool align_center, int truncate_width) {
    // Measure the text size
    int text_width, text_height;
    measureText(font, text, &text_width, &text_height);

    // Render the text into a surface
    SDL_Color fg;
    fg.r = color & 0xFF;
    fg.g = (color >> 8) & 0xFF;
    fg.b = (color >> 16) & 0xFF;
    fg.a = (color >> 24) & 0xFF;

    SDL_Surface *text_surface = TTF_RenderUTF8_Blended(font, text.c_str(), fg);

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

    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(ui()->renderer, text_surface);
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

    drawTextureClipped(text_texture, destination_rect, clip);
    SDL_DestroyTexture(text_texture);
}

void SUIGraphics::drawTextClipped(TTF_Font *font, std::string text, const SUIRect &bounds, const SUIRect &clip, uint32_t color, bool align_center) {
    // Measure the text size
    int text_width, text_height;
    measureText(font, text, &text_width, &text_height);

    // Render the text into a surface
    SDL_Color fg;
    fg.r = color & 0xFF;
    fg.g = (color >> 8) & 0xFF;
    fg.b = (color >> 16) & 0xFF;
    fg.a = (color >> 24) & 0xFF;

    SDL_Surface *text_surface = TTF_RenderUTF8_Blended(font, text.c_str(), fg);
    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(ui()->renderer, text_surface);
    SDL_FreeSurface(text_surface);

    // Render the surface at a specific location
    SUIRect global_bounds = offsetGlobal(bounds);
    SUIRect clip_rect = clipBounds(global_bounds, clip);
    SUIRect destination_rect;

    if (align_center)
    {
        destination_rect.x = bounds.x + bounds.w/2 - text_width/2;
        destination_rect.y = bounds.y + bounds.h/2 - text_height/2;
        destination_rect.w = text_width;
        destination_rect.h = text_height;
    }
    else
    {
        destination_rect = bounds;
    }

    drawTextureClipped(text_texture, destination_rect, clip_rect);
    SDL_DestroyTexture(text_texture);
}

SUIRect SUIGraphics::clipBounds(int x, int y, int width, int height, const SUIRect &clip) {
    return clipBounds({x, y, width, height}, clip);
}

SUIRect SUIGraphics::clipBounds(const SUIRect &bounds, const SUIRect &clip) {
    int x1 = bounds.x,
        y1 = bounds.y,
        x2 = bounds.x + bounds.w,
        y2 = bounds.y + bounds.h;

    if (x1 < clip.x) {
        x1 = clip.x;
    }

    if (x2 > (clip.x + clip.w)) {
        x2 = clip.x + clip.w;
    }

    if (y1 < clip.y) {
        y1 = clip.y;
    }

    if (y2 > (clip.y + clip.h)) {
        y2 = clip.y + clip.h;
    }

    return {x1, y1, x2 - x1, y2 - y1};
}

uint32_t SUIGraphics::interpolate(uint32_t a, uint32_t b, double t) {
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

SUIRect SUIGraphics::offsetGlobal(const SUIRect &bounds) {
    const SUIRect &offset = element()->globalBounds();

    return {
        bounds.x + offset.x,
        bounds.y + offset.y,
        bounds.w,
        bounds.h
    };
}

SUI *SUIGraphics::ui() {
    return element_->ui();
}

SUIElement *SUIGraphics::element() {
    return element_;
}