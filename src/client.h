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

#include "xml.h"

#include "limelight-common/Limelight.h"

#include "stdbool.h"

void client_init(const char* serverAddress);
void client_start_app(STREAM_CONFIGURATION *config, const char* serverAddress, int appId, bool sops, bool localaudio);
struct app_list* client_applist(const char* serverAddress);
int client_get_app_id(const char* serverAddress, char* name);
void client_pair(const char *address);
