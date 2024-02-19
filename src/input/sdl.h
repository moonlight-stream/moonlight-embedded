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

#include <stdbool.h>
#include <SDL.h>

extern int sdl_gamepads;

void sdlinput_init(char* mappings);
int sdlinput_handle_event(SDL_Window* window, SDL_Event* event);
void sdlinput_rumble(unsigned short controller_id, unsigned short low_freq_motor, unsigned short high_freq_motor);
void sdlinput_rumble_triggers(unsigned short controller_id, unsigned short left_trigger, unsigned short right_trigger);
void sdlinput_set_motion_event_state(unsigned short controller_id, unsigned char motion_type, unsigned short report_rate_hz);
void sdlinput_set_controller_led(unsigned short controller_id, unsigned char r, unsigned char g, unsigned char b);
