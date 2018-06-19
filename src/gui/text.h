#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <switch.h>

#include "gui.h"

#define TRUNCATE_FADE_WIDTH 15

int text_ascent(TTF_Font *font);
void text_measure(TTF_Font *font, const char *text, int *width, int *height);
void text_draw(TTF_Font *font, const char *text, int x, int y, uint32_t color, bool align_center, int truncate_width);
