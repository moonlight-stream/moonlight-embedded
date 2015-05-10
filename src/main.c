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

static void applist(const char* address) {
  struct app_list* list = client_applist(address);
  for (int i = 1;list != NULL;i++) {
    printf("%d. %s\n", i, list->name);
    list = list->next;
  }
}

static void stream(STREAM_CONFIGURATION* config, const char* address, const char* app) {
  int appId = client_get_app_id(address, app);
  if (appId<0) {
    printf("Can't find app %s\n", app);
    exit(-1);
  }

  client_start_app(config, address, appId);

  struct in_addr addr;
  inet_aton(address, &addr);
  LiStartConnection(addr.s_addr, config, &connection_callbacks, &decoder_callbacks, &audio_callbacks, &platform_callbacks, NULL, 0, 4);

  input_loop();

  LiStopConnection();
}

static void help() {
  printf("Usage: moonlight action [options] host\n\n");
  printf(" Actions\n\n");
  printf("\tpair\t\t\tPair device with computer\n");
  printf("\tstream\t\t\tStream computer to device\n");
  printf("\tlist\t\t\tList available games and applications\n");
  printf("\thelp\t\t\tShow this help\n\n");
  printf(" Streaming options\n\n");
  printf("\t-720\t\t\tUse 1280x720 resolution [default]\n");
  printf("\t-1080\t\t\tUse 1920x1080 resolution\n");
  printf("\t-width <width>\t\tHorizontal resolution (default 1280)\n");
  printf("\t-height <height>\tVertical resolution (default 720)\n");
  printf("\t-30fps\t\t\tUse 30fps\n");
  printf("\t-60fps\t\t\tUse 60fps [default]\n");
  printf("\t-bitrate <bitrate>\tSpecify the bitrate in Kbps\n");
  printf("\t-packetsize <size>\tSpecify the maximum packetsize in bytes\n");
  printf("\t-app <app>\t\tName of app to stream\n");
  printf("\t-input <device>\t\tUse <device> as input. Can be used multiple times\n");
  printf("\t-mapping <file>\t\tUse <file> as gamepad mapping configuration file (use before -input)\n");
  exit(0);
}

int main(int argc, char* argv[]) {
  STREAM_CONFIGURATION config;
  config.width = 1280;
  config.height = 720;
  config.fps = 60;
  config.bitrate = 8000;
  config.packetSize = 1024;

  static struct option long_options[] = {
    {"720", no_argument, 0, 'a'},
    {"1080", no_argument, 0, 'b'},
    {"width", required_argument, 0, 'c'},
    {"height", required_argument, 0, 'd'},
    {"30fps", no_argument, 0, 'e'},
    {"60fps", no_argument, 0, 'f'},
    {"bitrate", required_argument, 0, 'g'},
    {"packetsize", required_argument, 0, 'h'},
    {"app", required_argument, 0, 'i'},
    {"input", required_argument, 0, 'j'},
    {"mapping", required_argument, 0, 'k'},
    {0, 0, 0, 0},
  };

  char* app = "Steam";
  char* action = NULL;
  char* address = NULL;
  char* mapping = NULL;
  int option_index = 0;
  int c;
  while ((c = getopt_long_only (argc, argv, "-abc:d:efg:h:i:j:", long_options, &option_index)) != -1) {
    switch (c) {
    case 'a':
      config.width = 720;
      config.height = 1280;
      break;
    case 'b':
      config.width = 1080;
      config.height = 1920;
      break;
    case 'c':
      config.width = atoi(optarg);
      break;
    case 'd':
      config.height = atoi(optarg);
      break;
    case 'e':
      config.fps = 30;
      break;
    case 'f':
      config.fps = 60;
      break;
    case 'g':
      config.bitrate = atoi(optarg);
      break;
    case 'h':
      config.packetSize = atoi(optarg);
      break;
    case 'i':
      app = optarg;
      break;
    case 'j':
      input_create(optarg, mapping);
      break;
    case 'k':
      mapping = optarg;
      break;
    case 1:
      if (action == NULL)
        action = optarg;
      else if (address == NULL)
        address = optarg;
      else {
        perror("Too many options");
        exit(-1);
      }
    }
  }

  if (action == NULL || strcmp("help", action) == 0)
    help();

  if (address == NULL) {
    perror("No address given");
    exit(-1);
  }

  client_init(address);

  if (strcmp("applist", action) == 0)
    applist(address);
  else if (strcmp("stream", action) == 0)
    stream(&config, address, app);
  else if (strcmp("pair", action) == 0)
    client_pair(address);
  else
    fprintf(stderr, "%s is not a valid actions\n", action);
}
