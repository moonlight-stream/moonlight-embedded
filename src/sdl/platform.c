/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2015 Iwan Timmer
 *
 * Moonlight is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Moonlight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Moonlight; if not, see <http://www.gnu.org/licenses/>.
 */

#include "sdl.h"
#include "input.h"

#include "limelight-common/Limelight.h"

#include <stdbool.h>

static bool done;
static int fullscreen_flags;

SDL_Window *sdl_window;

void sdl_init(int width, int height) {
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
    fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
    exit(1);
  }

  sdl_window = SDL_CreateWindow("Moonlight", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);
  if(!sdl_window) {
    fprintf(stderr, "SDL: could not create window - exiting\n");
    exit(1);
  }
  SDL_SetRelativeMouseMode(SDL_TRUE);
  sdlinput_init();
}

void sdl_loop() {
  SDL_Event event;
  while(!done && SDL_WaitEvent(&event)) {
    switch (sdlinput_handle_event(&event)) {
    case SDL_QUIT_APPLICATION:
      done = true;
      break;
    case SDL_TOGGLE_FULLSCREEN:
      fullscreen_flags ^= SDL_WINDOW_FULLSCREEN;
      SDL_SetWindowFullscreen(sdl_window, fullscreen_flags);
    case SDL_MOUSE_GRAB:
      SDL_SetRelativeMouseMode(SDL_TRUE);
      break;
    case SDL_MOUSE_UNGRAB:
      SDL_SetRelativeMouseMode(SDL_FALSE);
      break;
    default:
      if (event.type == SDL_QUIT)
        done = true;
    }
  }

  SDL_DestroyWindow(sdl_window);
  SDL_Quit();
}
