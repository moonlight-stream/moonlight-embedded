#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <switch.h>

#include "gui.h"

void measure_text(TTF_Font *font, const char *text, int *width, int *height);
void draw_text(TTF_Font *font, const char *text, int x, int y, uint32_t color, bool align_center);
