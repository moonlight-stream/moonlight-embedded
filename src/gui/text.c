#include "text.h"

void measure_text(TTF_Font *font, const char *text, int *width, int *height) {
    // Measure the text size
    TTF_SizeText(font, text, width, height);
}

void draw_text(TTF_Font *font, const char *text, int x, int y, uint32_t color, bool align_center) {
    // Measure the text size
    int textWidth, textHeight;
    TTF_SizeText(font, text, &textWidth, &textHeight);

    // Render the text into a surface
    SDL_Color fg;
    fg.r = color & 0xFF;
    fg.g = (color >> 8) & 0xFF;
    fg.b = (color >> 16) & 0xFF;
    fg.a = (color >> 24) & 0xFF;

    SDL_Surface *textSurface = TTF_RenderText_Blended(font, text, fg);
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(gui.renderer, textSurface);
    SDL_FreeSurface(textSurface);

    // Render the surface at a specific location
    SDL_Rect sourceRect = { 0, 0, textWidth, textHeight };
    SDL_Rect destinationRect;

    if (align_center) {
        destinationRect.x = x - textWidth/2;
        destinationRect.y = y - textHeight/2;
        destinationRect.w = textWidth;
        destinationRect.h = textHeight;
    }
    else {
      destinationRect.x = x;
      destinationRect.y = y;
      destinationRect.w = textWidth;
      destinationRect.h = textHeight;
    }

    SDL_RenderCopy(
        gui.renderer,
        textTexture,
        &sourceRect,
        &destinationRect
    );
}
