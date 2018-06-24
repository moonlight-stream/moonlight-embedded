#pragma once

#include "common.h"

#define TRUNCATE_FADE_WIDTH 15

int text_ascent(TTF_Font *font);
void measure_text(TTF_Font *font, const char *text, int *width, int *height);
void draw_text(TTF_Font *font, const char *text, int x, int y, uint32_t color, bool align_center, int truncate_width);
void draw_clipped_text(TTF_Font *font, const char *text, int x, int y, Rect *clip, uint32_t color, bool align_center, int truncate_width);
