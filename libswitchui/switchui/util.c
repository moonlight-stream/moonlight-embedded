#include "util.h"

uint64_t milliseconds() {
  return svcGetSystemTick() / 19200;
}

uint32_t interpolate(uint32_t a, uint32_t b, double t) {
  uint8_t
      ar = (a & 0xff), br = (b & 0xff),
      ag = ((a >> 8) & 0xff), bg = ((b >> 8) & 0xff),
      ab = ((a >> 16) & 0xff), bb = ((b >> 16) & 0xff),
      aa = ((a >> 24) & 0xff), ba = ((b >> 24) & 0xff);

  return RGBA8(
      (uint8_t)(ar + t*(br - ar)),
      (uint8_t)(ag + t*(bg - ag)),
      (uint8_t)(ab + t*(bb - ab)),
      (uint8_t)(aa + t*(ba - aa))
  );
}

SUIRect sui_intersect_bounds_clip(int x, int y, int width, int height, SUIRect *clip) {
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

  SUIRect intersect = {
    .x = x1,
    .y = y1,
    .w = x2 - x1,
    .h = y2 - y1
  };
  return intersect;
}

bool sui_rect_contains_point(SUIRect *rect, int x, int y) {
  return
      x >= rect->x &&
      x <= (rect->x + rect->w) &&
      y >= rect->y &&
      y <= (rect->y + rect->h);
}
