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

#include "client.h"
#include "connection.h"
#include "video.h"
#include "audio.h"
#include "input.h"

#include "limelight-common/Limelight.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/rand.h>
#include <getopt.h>

PLATFORM_CALLBACKS platform_callbacks = {
  .threadStart = NULL,
  .debugPrint = NULL,
};

int main(int argc, char* argv[]) {
  STREAM_CONFIGURATION config;
  config.width = 1280;
  config.height = 720;
  config.fps = 60;
  config.bitrate = 8000;
  config.packetSize = 1024;

  client_init(argv[1]);

  int appId = client_get_app_id(address, argv[2]);
  if (appId<0) {
    printf("Can't find app %s\n", app);
    exit(-1);
  }

  client_start_app(config, argv[1], appId);

  input_create(argv[3]);

  struct in_addr addr;
  inet_aton(address, &addr);
  LiStartConnection(addr.s_addr, config, &connection_callbacks, &decoder_callbacks, &audio_callbacks, &platform_callbacks, NULL, 0, 4);

  input_loop();

  LiStopConnection();
}
