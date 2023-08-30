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

#pragma once

#include <stdbool.h>

struct mapping {
  char guid[33];
  char platform[33];
  char name[257];

  bool reverse_leftx, reverse_lefty;
  bool reverse_rightx, reverse_righty;
  char halfaxis_lefttrigger, halfaxis_righttrigger;
  char halfaxis_dpright, halfaxis_dpleft;
  char halfaxis_dpup, halfaxis_dpdown;

  /* abs_leftx must be the first member of the list of mapping indices! */
  short abs_leftx, abs_lefty;
  short abs_rightx, abs_righty;

  short hat_dpright, hat_dpleft, hat_dpup, hat_dpdown;
  short hat_dir_dpright, hat_dir_dpleft, hat_dir_dpup, hat_dir_dpdown;
  short btn_dpup, btn_dpdown, btn_dpleft, btn_dpright;
  short abs_dpright, abs_dpleft, abs_dpup, abs_dpdown;

  short btn_a, btn_x, btn_y, btn_b;
  short btn_back, btn_start, btn_guide;
  short btn_leftstick, btn_rightstick;
  short btn_leftshoulder, btn_rightshoulder;
  short btn_misc1;
  short btn_paddle1, btn_paddle2, btn_paddle3, btn_paddle4;
  short btn_touchpad;

  short abs_lefttrigger, abs_righttrigger;
  short btn_lefttrigger, btn_righttrigger;

  /* next must be the last member after the list of mapping indices! */
  struct mapping* next;
};

struct mapping* mapping_parse(char* mapping);
struct mapping* mapping_load(char* fileName, bool verbose);
void mapping_print(struct mapping*);
