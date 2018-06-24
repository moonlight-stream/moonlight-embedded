#pragma once

#include "common.h"

uint64_t milliseconds();
uint32_t interpolate(uint32_t a, uint32_t b, double t);

SUIRect sui_intersect_bounds_clip(int x, int y, int width, int height, SUIRect *clip);
bool sui_rect_contains_point(SUIRect *rect, int x, int y);
