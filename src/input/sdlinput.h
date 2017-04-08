/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2015-2017 Iwan Timmer
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

#include <stdbool.h>
#include <SDL.h>

static const short keyCodes[] = {
  0x14, //SDLK_CAPSLOCK
  0x70, //SDLK_F1
  0x71, //SDLK_F2
  0x72, //SDLK_F3
  0x73, //SDLK_F4
  0x74, //SDLK_F5
  0x75, //SDLK_F6
  0x76, //SDLK_F7
  0x77, //SDLK_F8
  0x78, //SDLK_F9
  0x79, //SDLK_F10
  0x7A, //SDLK_F11
  0x7B, //SDLK_F12
  0, //SDLK_PRINTSCREEN
  0x91, //SDLK_SCROLLLOCK
  0x13, //SDLK_PAUSE
  0x9B, //SDLK_INSERT
  0x24, //SDLK_HOME
  0x21, //SDLK_PAGEUP
  0, //Not used
  0x23, //SDLK_END
  0x22, //SDLK_PAGEDOWN
  0x27, //SDLK_RIGHT
  0x25, //SDLK_LEFT
  0x28, //SDLK_DOWN
  0x26, //SDLK_UP
};

void sdlinput_init(char* mappings);
int sdlinput_handle_event(SDL_Event* event);

#endif /* HAVE_SDL */
