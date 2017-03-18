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

#include <SDL.h>

#include <stdbool.h>

#define SDL_NOTHING 0
#define SDL_QUIT_APPLICATION 1
#define SDL_MOUSE_GRAB 2
#define SDL_MOUSE_UNGRAB 3
#define SDL_TOGGLE_FULLSCREEN 4

#define SDL_CODE_FRAME 0

#define SDL_BUFFER_FRAMES 2

void sdl_init(int width, int height, bool fullscreen);
void sdl_loop();

SDL_mutex *mutex;
int sdlCurrentFrame, sdlNextFrame;

#endif /* HAVE_SDL */
