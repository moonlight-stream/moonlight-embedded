#include "gui.h"
#include "../input/switch.h"

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL2_rotozoom.h>
#include <time.h>

static int load_shared_fonts() {
  Result rc = plGetSharedFontByType(&gui.fontData, PlSharedFontType_Standard);
  if (R_FAILED(rc)) {
    fprintf(stderr, "[GUI] Could not load Switch shared font\n");
    return -1;
  }

  gui.fontSmall = TTF_OpenFontRW(SDL_RWFromMem(gui.fontData.address, gui.fontData.size), 1, 18);
  gui.fontNormal = TTF_OpenFontRW(SDL_RWFromMem(gui.fontData.address, gui.fontData.size), 1, 22);
  gui.fontHeading = TTF_OpenFontRW(SDL_RWFromMem(gui.fontData.address, gui.fontData.size), 1, 28);
  gui.fontMassive = TTF_OpenFontRW(SDL_RWFromMem(gui.fontData.address, gui.fontData.size), 1, 64);

  if (!gui.fontSmall || !gui.fontNormal || !gui.fontHeading || !gui.fontMassive) {
    fprintf(stderr, "[GUI] Could not load font into SDL: %s\n", TTF_GetError());
    return -1;
  }

  return 0;
}

int gui_init() {
  memset(&gui, 0, sizeof(gui));

  if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0) {
    fprintf(stderr, "[GUI] Could not initialize SDL: %s\n", SDL_GetError());
    return -1;
  }

  int flags = IMG_INIT_PNG;
  if (IMG_Init(flags) != flags) {
    fprintf(stderr, "[GUI] Could not initialize SDL image library: %s\n", IMG_GetError());
  }

  if (TTF_Init() < 0) {
    fprintf(stderr, "[GUI] Could not initialize SDL font library: %s\n", TTF_GetError());
    return -1;
  }

  if (SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &gui.window, &gui.renderer) < 0) {
    fprintf(stderr, "[GUI] Could not create SDL window and renderer: %s\n", SDL_GetError());
    return -1;
  }

  SDL_GetWindowSize(gui.window, &gui.width, &gui.height);

  if (switch_input_init() < 0) {
    return -1;
  }

  if (load_shared_fonts() < 0) {
    return -1;
  }

  if (gui_main_init() < 0) {
    return -1;
  }

  return 0;
}

void gui_cleanup() {
  gui_main_cleanup();

  if (gui.fontSmall) { TTF_CloseFont(gui.fontSmall); }
  if (gui.fontNormal) { TTF_CloseFont(gui.fontNormal); }
  if (gui.fontHeading) { TTF_CloseFont(gui.fontHeading); }
  if (gui.fontMassive) { TTF_CloseFont(gui.fontMassive); }

  if (gui.renderer) {
    SDL_DestroyRenderer(gui.renderer);
    gui.renderer = NULL;
  }

  if (gui.window) {
    SDL_DestroyWindow(gui.window);
    gui.window = NULL;
  }

  switch_input_cleanup();

  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
}

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

SDL_Texture *load_png(const void *data, size_t size) {
  return load_png_rescale(data, size, -1, -1);
}

SDL_Texture *load_png_rescale(const void *data, size_t size, int width, int height) {
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

    if (!scaled) {
      fprintf(stderr, "[GUI] Could not resize PNG of size (%d,%d) to size (%d,%d)\n");
      return NULL;
    }
  }

  texture = SDL_CreateTextureFromSurface(gui.renderer, surface);
  if (!texture) {
    fprintf(stderr, "[GUI] Could not create PNG texture from surface: %s\n", SDL_GetError());
    return NULL;
  }
  SDL_FreeSurface(surface);

  return texture;
}

void draw_texture(SDL_Texture *texture, int x, int y, int w, int h) {
  SDL_Rect dst;
  dst.x = x;
  dst.y = y;
  dst.w = w;
  dst.h = h;
  SDL_RenderCopy(gui.renderer, texture, NULL, &dst);
}

void draw_clipped_texture(SDL_Texture *texture, int x, int y, int w, int h, Rect *clip) {
  Rect clippedDestination = intersect_bounds_clip(x, y, w, h, clip);
  Rect clippedSource = {
    .x = clippedDestination.x - x,
    .y = clippedDestination.y - y,
    .width = clippedDestination.width,
    .height = clippedDestination.height
  };

  SDL_RenderCopy(
      gui.renderer,
      texture,
      &clippedSource,
      &clippedDestination
  );
}

void draw_clipped_box_bounds(Rect *bounds, Rect *clip, uint32_t color) {
  draw_clipped_box(bounds->x, bounds->y, bounds->width, bounds->height, clip, color);
}

void draw_clipped_box(int x, int y, int width, int height, Rect *clip, uint32_t color) {
  Rect in = intersect_bounds_clip(x, y, width, height, clip);

  // Only draw the box if both dimensions are positive
  if (in.width > 0 && in.height > 0) {
    boxColor(gui.renderer, in.x, in.y, in.x + in.width, in.y + in.height, color);
  }
}

void draw_clipped_rectangle_bounds(Rect *bounds, Rect *clip, uint32_t color) {
  draw_clipped_rectangle(bounds->x, bounds->y, bounds->width, bounds->height, clip, color);
}

void draw_clipped_rectangle(int x, int y, int width, int height, Rect *clip, uint32_t color) {
  Rect in = intersect_bounds_clip(x, y, width, height, clip);

  // Only draw the rectangle if both dimensions are positive
  if (in.width > 0 && in.height > 0) {
    // Top edge
    if (y == in.y) {
      hlineColor(gui.renderer, in.x, in.x + in.width, y, color);
    }

    // Bottom edge
    if ((y + height) == (in.y + in.height)) {
      hlineColor(gui.renderer, in.x, in.x + in.width, in.y + in.height, color);
    }

    // Left edge
    if (x == in.x) {
      vlineColor(gui.renderer, x, in.y, in.y + in.height, color);
    }

    // Right edge
    if ((x + width) == (in.x + in.width)) {
      vlineColor(gui.renderer, in.x + in.width, in.y, in.y + in.height, color);
    }
  }
}

Rect get_clip(Element *element) {
  if (element->_scene) {
    return element->_scene->clip;
  }

  Rect clip = {
    .x = 0,
    .y = 0,
    .width = gui.width,
    .height = gui.height
  };
  return clip;
}

Rect intersect_bounds_clip(int x, int y, int width, int height, Rect *clip) {
  int x1 = x,
      y1 = y,
      x2 = x + width,
      y2 = y + height;

  if (x1 < clip->x) {
    x1 = clip->x;
  }

  if (x2 > (clip->x + clip->width)) {
    x2 = clip->x + clip->width;
  }

  if (y1 < clip->y) {
    y1 = clip->y;
  }

  if (y2 > (clip->y + clip->height)) {
    y2 = clip->y + clip->height;
  }

  Rect intersect = {
    .x = x1,
    .y = y1,
    .width = x2 - x1,
    .height = y2 - y1
  };
  return intersect;
}
