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

#include "config.h"
#include "platform.h"

#include <Limelight.h>
#include <client.h>
#include <errors.h>

#include <stdbool.h>

int pair_check(PSERVER_DATA server);
size_t get_app_list(PSERVER_DATA server, PAPP_LIST *list);
int get_app_id(PSERVER_DATA server, const char *name);

void stream_start(PSERVER_DATA server, PCONFIGURATION config, enum platform system);
void stream_stop(enum platform system);

extern CONNECTION_LISTENER_CALLBACKS connection_callbacks;
extern bool connection_debug;
