#include "ui.h"

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL2_rotozoom.h>
#include <time.h>

#include "button_a_png.h"
#include "button_b_png.h"

static int shared_fonts_init() {
  Result rc = plGetSharedFontByType(&ui.fontData, PlSharedFontType_Standard);
  if (R_FAILED(rc)) {
    fprintf(stderr, "[GUI] Could not load Switch shared font\n");
    return -1;
  }

  ui.fontSmall = TTF_OpenFontRW(SDL_RWFromMem(ui.fontData.address, ui.fontData.size), 1, 18);
  ui.fontNormal = TTF_OpenFontRW(SDL_RWFromMem(ui.fontData.address, ui.fontData.size), 1, 22);
  ui.fontHeading = TTF_OpenFontRW(SDL_RWFromMem(ui.fontData.address, ui.fontData.size), 1, 28);
  ui.fontMassive = TTF_OpenFontRW(SDL_RWFromMem(ui.fontData.address, ui.fontData.size), 1, 64);

  if (!ui.fontSmall || !ui.fontNormal || !ui.fontHeading || !ui.fontMassive) {
    fprintf(stderr, "[GUI] Could not load font into SDL: %s\n", TTF_GetError());
    return -1;
  }

  return 0;
}

static void shared_fonts_cleanup() {
  if (ui.fontSmall) { TTF_CloseFont(ui.fontSmall); }
  if (ui.fontNormal) { TTF_CloseFont(ui.fontNormal); }
  if (ui.fontHeading) { TTF_CloseFont(ui.fontHeading); }
  if (ui.fontMassive) { TTF_CloseFont(ui.fontMassive); }
}

static int toolbar_textures_init() {
  ui.buttonATexture = sui_load_png(button_a_png, button_a_png_size);
  if (!ui.buttonATexture) {
    fprintf(stderr, "[GUI] Could not load button A image: %s\n", SDL_GetError());
    return -1;
  }
  SDL_QueryTexture(ui.buttonATexture, NULL, NULL, &ui.buttonAWidth, &ui.buttonAHeight);

  ui.buttonBTexture = sui_load_png(button_b_png, button_b_png_size);
  if (!ui.buttonBTexture) {
    fprintf(stderr, "[GUI] Could not load button B image: %s\n", SDL_GetError());
    return -1;
  }
  SDL_QueryTexture(ui.buttonBTexture, NULL, NULL, &ui.buttonBWidth, &ui.buttonBHeight);

  return 0;
}

static void toolbar_textures_cleanup() {
  if (ui.buttonATexture) {
    SDL_DestroyTexture(ui.buttonATexture);
    ui.buttonATexture = NULL;
  }

  if (ui.buttonBTexture) {
    SDL_DestroyTexture(ui.buttonBTexture);
    ui.buttonBTexture = NULL;
  }
}

int sui_init() {
  memset(&ui, 0, sizeof(ui));

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

  if (SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &ui.window, &ui.renderer) < 0) {
    fprintf(stderr, "[GUI] Could not create SDL window and renderer: %s\n", SDL_GetError());
    return -1;
  }

  SDL_GetWindowSize(ui.window, &ui.width, &ui.height);

  if (sui_input_init() < 0) { return -1; }
  if (shared_fonts_init() < 0) { return -1; }
  if (toolbar_textures_init() < 0) { return -1; }

  return 0;
}

void sui_cleanup() {
  shared_fonts_cleanup();
  toolbar_textures_cleanup();
  sui_input_cleanup();

  if (ui.renderer) {
    SDL_DestroyRenderer(ui.renderer);
    ui.renderer = NULL;
  }

  if (ui.window) {
    SDL_DestroyWindow(ui.window);
    ui.window = NULL;
  }

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

SDL_Texture *sui_load_png(const void *data, size_t size) {
  return sui_load_png_rescale(data, size, -1, -1);
}

SDL_Texture *sui_load_png_rescale(const void *data, size_t size, int width, int height) {
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

  texture = SDL_CreateTextureFromSurface(ui.renderer, surface);
  if (!texture) {
    fprintf(stderr, "[GUI] Could not create PNG texture from surface: %s\n", SDL_GetError());
    return NULL;
  }
  SDL_FreeSurface(surface);

  return texture;
}

void sui_draw_bottom_toolbar(int count, ...) {
  va_list args;
  va_start(args, count);

  int baseHeight = sui_text_ascent(ui.fontNormal);
  int offsetX = ui.width - SUI_MARGIN_SIDE - SUI_MARGIN_TOOLBAR_SIDE;
  int offsetY = ui.height - SUI_MARGIN_BOTTOM + (SUI_MARGIN_BOTTOM - baseHeight)/2;

  for (int i = 0; i < count; i++) {
    // Obtain the button arguments
    char *text = va_arg(args, char *);
    enum SUIToolbarAction action = va_arg(args, enum SUIToolbarAction);

    SDL_Texture *iconTexture;
    int iconWidth, iconHeight;

    switch (action) {
      case SUIToolbarActionA:
        iconTexture = ui.buttonATexture;
        iconWidth = ui.buttonAWidth;
        iconHeight = ui.buttonAHeight;
        break;

      case SUIToolbarActionB:
        iconTexture = ui.buttonBTexture;
        iconWidth = ui.buttonBWidth;
        iconHeight = ui.buttonBHeight;
        break;

       default:
        continue;
    }

    // Measure the size of this particular label
    int textWidth;
    sui_measure_text(ui.fontNormal, text, &textWidth, NULL);

    // Draw the text and icon
    sui_draw_text(ui.fontNormal, text, offsetX - textWidth, offsetY, SUI_COLOR_DARK, false, -1);
    sui_draw_texture(iconTexture,
                 offsetX - textWidth - SUI_MARGIN_BETWEEN_TOOLBAR_ICON_TEXT - iconWidth,
                 ui.height - SUI_MARGIN_BOTTOM + (SUI_MARGIN_BOTTOM - iconHeight)/2,
                 iconWidth,
                 iconHeight);

    offsetX = offsetX - textWidth - SUI_MARGIN_BETWEEN_TOOLBAR_ICON_TEXT - iconWidth - SUI_MARGIN_BETWEEN_TOOLBAR_BUTTONS;
  }


  // Draw the bottom separator
  hlineColor(ui.renderer, SUI_MARGIN_SIDE, ui.width - SUI_MARGIN_SIDE, ui.height - SUI_MARGIN_BOTTOM, SUI_COLOR_DARK);

  va_end(args);
}

void sui_draw_top_header(const char *text) {
  // Draw the top separator
  hlineColor(ui.renderer, SUI_MARGIN_SIDE, ui.width - SUI_MARGIN_SIDE, SUI_MARGIN_TOP, SUI_COLOR_DARK);

  // Draw the text
  int textWidth, textHeight = sui_text_ascent(ui.fontHeading);
  sui_measure_text(ui.fontHeading, text, &textWidth, NULL);
  sui_draw_text(ui.fontHeading,
            text,
            SUI_MARGIN_SIDE + SUI_MARGIN_TOOLBAR_SIDE,
            (SUI_MARGIN_TOP - textHeight)/2 + 10,
            SUI_COLOR_DARK,
            false,
            -1);
}

void sui_draw_texture(SDL_Texture *texture, int x, int y, int w, int h) {
  SUIRect dst;
  dst.x = x;
  dst.y = y;
  dst.w = w;
  dst.h = h;
  SDL_RenderCopy(ui.renderer, texture, NULL, &dst);
}

void sui_draw_clipped_texture(SDL_Texture *texture, int x, int y, int w, int h, SUIRect *clip) {
  SUIRect clippedDestination = sui_intersect_bounds_clip(x, y, w, h, clip);
  SUIRect clippedSource = {
    .x = clippedDestination.x - x,
    .y = clippedDestination.y - y,
    .w = clippedDestination.w,
    .h = clippedDestination.h
  };

  SDL_RenderCopy(
      ui.renderer,
      texture,
      &clippedSource,
      &clippedDestination
  );
}

void sui_draw_clipped_box_bounds(SUIRect *bounds, SUIRect *clip, uint32_t color) {
  sui_draw_clipped_box(bounds->x, bounds->y, bounds->w, bounds->h, clip, color);
}

void sui_draw_clipped_box(int x, int y, int width, int height, SUIRect *clip, uint32_t color) {
  SUIRect in = sui_intersect_bounds_clip(x, y, width, height, clip);

  // Only draw the box if both dimensions are positive
  if (in.w > 0 && in.h > 0) {
    boxColor(ui.renderer, in.x, in.y, in.x + in.w, in.y + in.h, color);
  }
}

void sui_draw_clipped_rectangle_bounds(SUIRect *bounds, SUIRect *clip, uint32_t color) {
  sui_draw_clipped_rectangle(bounds->x, bounds->y, bounds->w, bounds->h, clip, color);
}

void sui_draw_clipped_rectangle(int x, int y, int width, int height, SUIRect *clip, uint32_t color) {
  SUIRect in = sui_intersect_bounds_clip(x, y, width, height, clip);

  // Only draw the rectangle if both dimensions are positive
  if (in.w > 0 && in.h > 0) {
    // Top edge
    if (y == in.y) {
      hlineColor(ui.renderer, in.x, in.x + in.w, y, color);
    }

    // Bottom edge
    if ((y + height) == (in.y + in.h)) {
      hlineColor(ui.renderer, in.x, in.x + in.w, in.y + in.h, color);
    }

    // Left edge
    if (x == in.x) {
      vlineColor(ui.renderer, x, in.y, in.y + in.h, color);
    }

    // Right edge
    if ((x + width) == (in.x + in.w)) {
      vlineColor(ui.renderer, in.x + in.w, in.y, in.y + in.h, color);
    }
  }
}

SUIRect sui_get_clip(SUIElement *element) {
  if (element->_scene) {
    return element->_scene->clip;
  }

  SUIRect clip = {
    .x = 0,
    .y = 0,
    .w = ui.width,
    .h = ui.height
  };
  return clip;
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
