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

#ifdef HAVE_SDL

#include "sdl.h"
#include "input/sdlinput.h"

#include "limelight-common/Limelight.h"

#include <stdbool.h>
#include <SDL.h>

static bool done;

void sdl_loop() {
  SDL_InitSubSystem(SDL_INIT_EVENTS);
  SDL_ShowCursor(SDL_DISABLE);

  SDL_Event event;

  while(!done && SDL_WaitEvent(&event)) {
    if (!sdlinput_handle_event(&event))
      done = false;
    else if (event.type == SDL_QUIT)
      done = true;
  }
}

#endif /* HAVE_SDL */
