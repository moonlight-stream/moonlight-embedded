#pragma once

#include "common.h"

#define SUI_TRUNCATE_FADE_WIDTH 15

int sui_text_ascent(TTF_Font *font);
void sui_measure_text(TTF_Font *font, const char *text, int *width, int *height);
void sui_draw_text(TTF_Font *font, const char *text, int x, int y, uint32_t color, bool align_center, int truncate_width);
void sui_draw_clipped_text(TTF_Font *font, const char *text, int x, int y, SUIRect *clip, uint32_t color, bool align_center, int truncate_width);
