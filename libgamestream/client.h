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

#pragma once

#include "xml.h"

#include "limelight-common/Limelight.h"

#include <stdbool.h>

typedef struct _SERVER_DATA {
  const char* address;
  bool paired;
  bool supports4K;
  int currentGame;
  int serverMajorVersion;
} SERVER_DATA, *PSERVER_DATA;

int gs_init(PSERVER_DATA server, const char *keyDirectory);
int gs_start_app(PSERVER_DATA server, PSTREAM_CONFIGURATION config, int appId, bool sops, bool localaudio);
int gs_applist(PSERVER_DATA server, PAPP_LIST *app_list);
int gs_pair(PSERVER_DATA server, char* pin);
int gs_quit_app(PSERVER_DATA server);
