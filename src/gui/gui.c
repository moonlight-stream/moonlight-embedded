#include "gui.h"

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

static int load_shared_fonts() {
  Result rc = plGetSharedFontByType(&gui.fontData, PlSharedFontType_Standard);
  if (R_FAILED(rc)) {
    fprintf(stderr, "[GUI] Could not load Switch shared font\n");
    return -1;
  }

  gui.fontNormal = TTF_OpenFontRW(SDL_RWFromMem(gui.fontData.address, gui.fontData.size), 1, 22);
  gui.fontHeading = TTF_OpenFontRW(SDL_RWFromMem(gui.fontData.address, gui.fontData.size), 1, 28);
  gui.fontMassive = TTF_OpenFontRW(SDL_RWFromMem(gui.fontData.address, gui.fontData.size), 1, 48);

  if (!gui.fontNormal || !gui.fontHeading || !gui.fontMassive) {
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

  if (load_shared_fonts() < 0) {
    return -1;
  }

  if (gui_main_init() < 0) {
    return -1;
  }

  if (gui_stream_init() < 0) {
    return -1;
  }

  return 0;
}

void gui_cleanup() {
  gui_stream_cleanup();
  gui_main_cleanup();

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

  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
}

SDL_Texture *load_png(void *data, size_t size) {
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

  texture = SDL_CreateTextureFromSurface(gui.renderer, surface);
  if (!texture) {
    fprintf(stderr, "[GUI] Could not create PNG texture from surface: %s\n", SDL_GetError());
    return NULL;
  }

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
