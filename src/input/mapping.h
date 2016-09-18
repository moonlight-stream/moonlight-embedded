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

#include <stdint.h>
#include <stdbool.h>

struct mapping {
  short abs_x, abs_y, abs_z;
  short abs_rx, abs_ry, abs_rz;

  bool reverse_x, reverse_y;
  bool reverse_rx, reverse_ry;

  short abs_deadzone;

  short abs_dpad_x, abs_dpad_y;
  bool reverse_dpad_x, reverse_dpad_y;

  uint32_t btn_south, btn_east, btn_north, btn_west;
  uint32_t btn_select, btn_start, btn_mode;
  uint32_t btn_thumbl, btn_thumbr;
  uint32_t btn_tl, btn_tr, btn_tl2, btn_tr2;

  uint32_t btn_dpad_up, btn_dpad_down, btn_dpad_left, btn_dpad_right;
};

void mapping_load(char* fileName, struct mapping* map);
void mapping_save(char* fileName, struct mapping* map);
