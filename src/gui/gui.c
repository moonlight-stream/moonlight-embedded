#include "gui.h"

int gui_init() {
  if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0) {
    fprintf(stderr, "[GUI] Could not initialize SDL: %s\n", SDL_GetError());
    return -1;
  }

  if (SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &gui.window, &gui.renderer) < 0) {
    fprintf(stderr, "[GUI] Could not create SDL window and renderer: %s\n", SDL_GetError());
    return -1;
  }

  SDL_GetWindowSize(gui.window, &gui.width, &gui.height);

  gui.streamTexture = SDL_CreateTexture(gui.renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, gui.width, gui.height);
  if (!gui.streamTexture) {
    fprintf(stderr, "[GUI] Could not create SDL texture for streaming: %s\n", SDL_GetError());
    return -1;
  }

  return 0;
}

void gui_cleanup() {
  if (gui.streamTexture) {
    SDL_DestroyTexture(gui.streamTexture);
    gui.streamTexture = NULL;
  }

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
