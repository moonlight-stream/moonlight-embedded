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

#include <stdbool.h>
#include <Limelight.h>

//.DISCONNECTED <-.
//      |         |
//      v         |
//    READY       |
//      |         |
//      v         |
//    PAIRED   ---|
//     | ^        |
//     v |        |
//  CONNECTED  ---|
//     | ^        |
//     v |        |
//  MINIMISED  ---'

enum {
  LI_DISCONNECTED,
  LI_READY,
  LI_PAIRED,
  LI_CONNECTED,
  LI_MINIMIZED
};

extern CONNECTION_LISTENER_CALLBACKS connection_callbacks;
extern int connection_failed_stage;
extern long connection_failed_stage_code;

int connection_reset();
int connection_paired();
int connection_minimize();
int connection_resume();
int connection_terminate();

bool connection_is_ready();
int connection_get_status();
