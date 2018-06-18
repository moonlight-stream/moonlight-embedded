#include "gui.h"

#include <SDL2/SDL_image.h>

int gui_init() {
  if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0) {
    fprintf(stderr, "[GUI] Could not initialize SDL: %s\n", SDL_GetError());
    return -1;
  }

  int flags = IMG_INIT_PNG;
  if (IMG_Init(flags) != flags) {
    fprintf(stderr, "[GUI] Could not initialize SDL image library: %s\n", IMG_GetError());
  }

  if (SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &gui.window, &gui.renderer) < 0) {
    fprintf(stderr, "[GUI] Could not create SDL window and renderer: %s\n", SDL_GetError());
    return -1;
  }

  SDL_GetWindowSize(gui.window, &gui.width, &gui.height);

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

  if (gui.renderer) {
    SDL_DestroyRenderer(gui.renderer);
    gui.renderer = NULL;
  }

  if (gui.window) {
    SDL_DestroyWindow(gui.window);
    gui.window = NULL;
  }

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
