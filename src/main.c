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
#include "discover.h"

#include "limelight-common/Limelight.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <openssl/rand.h>
#include <getopt.h>

#define MOONLIGHT_PATH "/moonlight/"
#define USER_PATHS ":~/.moonlight:./"

static void applist(const char* address) {
  struct app_list* list = client_applist(address);
  for (int i = 1;list != NULL;i++) {
    printf("%d. %s\n", i, list->name);
    list = list->next;
  }
}

static void stream(STREAM_CONFIGURATION* config, const char* address, const char* app, bool sops, bool localaudio) {
  int appId = client_get_app_id(address, app);
  if (appId<0) {
    fprintf(stderr, "Can't find app %s\n", app);
    exit(-1);
  }

  client_start_app(config, address, appId, sops, localaudio);

  video_init();

  LiStartConnection(address, config, &connection_callbacks, decoder_callbacks, &audio_callbacks, NULL, NULL, 0, client_get_server_version());

  input_loop();

  LiStopConnection();
}

static void help() {
  printf("Usage: moonlight action [options] host\n\n");
  printf(" Actions\n\n");
  printf("\tmap\t\t\tCreate mapping file for gamepad\n");
  printf("\tpair\t\t\tPair device with computer\n");
  printf("\tstream\t\t\tStream computer to device\n");
  printf("\tlist\t\t\tList available games and applications\n");
  printf("\tquit\t\t\tQuit the application or game being streamed\n");
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
  printf("\t-nosops\t\t\tDon't allow GFE to modify game settings\n");
  printf("\t-input <device>\t\tUse <device> as input. Can be used multiple times\n");
  printf("\t-mapping <file>\t\tUse <file> as gamepad mapping configuration file (use before -input)\n");
  printf("\t-audio <device>\t\tUse <device> as ALSA audio output device (default sysdefault)\n");
  printf("\t-localaudio\t\tPlay audio locally\n");
  exit(0);
}

char* get_path(char* name) {
  const char *xdg_data_dirs = getenv("XDG_DATA_DIRS");
  char *data_dirs;
  if (!xdg_data_dirs)
    data_dirs = "/usr/share:/usr/local/share:" USER_PATHS;
  else {
    data_dirs = malloc(strlen(xdg_data_dirs) + strlen(USER_PATHS) + 1);
    strcpy(data_dirs, xdg_data_dirs);
    strcpy(data_dirs+strlen(data_dirs), USER_PATHS);
  }

  char *path = malloc(strlen(data_dirs)+strlen(MOONLIGHT_PATH)+strlen(name)+1);
  if (path == NULL) {
    fprintf(stderr, "Not enough memory\n");
    exit(-1);
  }

  char* end;
  do {
    end = strstr(data_dirs, ":");
    int length = end != NULL?end - data_dirs:strlen(data_dirs);
    memcpy(path, data_dirs, length);
    if (path[0] == '/')
      sprintf(path+length, "%s%s", MOONLIGHT_PATH, name);
    else
      sprintf(path+length, "%s", name);

    if(access(path, F_OK) != -1)
      return path;

    data_dirs = end + 1;
  } while (end != NULL);

  free(path);
  return NULL;
}

static void pair_check(void) {
  if (!client_is_paired(NULL)) {
    fprintf(stderr, "You must pair with the PC first\n");
    exit(-1);
  }
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
    {"nosops", no_argument, 0, 'l'},
    {"audio", required_argument, 0, 'm'},
    {"localaudio", no_argument, 0, 'n'},
    {0, 0, 0, 0},
  };

  char* app = "Steam";
  char* action = NULL;
  char* address = NULL;
  char* mapping = get_path("mappings/default.conf");
  int option_index = 0;
  bool sops = true;
  bool localaudio = false;
  int c;
  while ((c = getopt_long_only(argc, argv, "-abc:d:efg:h:i:j:k:lm:n", long_options, &option_index)) != -1) {
    switch (c) {
    case 'a':
      config.width = 1280;
      config.height = 720;
      break;
    case 'b':
      config.width = 1920;
      config.height = 1080;
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
      mapping = get_path(optarg);
      break;
    case 'l':
      sops = false;
      break;
    case 'm':
      audio_device = optarg;
      break;
    case 'n':
      localaudio = true;
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
  else if (strcmp("map", action) == 0) {
    if (address == NULL) {
      perror("No filename for mapping");
      exit(-1);
    }
    input_init(mapping);
    input_map(address);
    exit(0);
  }

  if (address == NULL) {
    address = malloc(MAX_ADDRESS_SIZE);
    if (address == NULL) {
      perror("Not enough memory");
      exit(-1);
    }
    address[0] = 0;
    discover_server(address);
    if (address[0] == 0) {
      perror("Can't find server");
      exit(-1);
    }
  }

  client_init(address);

  if (strcmp("list", action) == 0) {
    pair_check();
    applist(address);
  } else if (strcmp("stream", action) == 0) {
    input_init(mapping);
    pair_check();
    stream(&config, address, app, sops, localaudio);
  } else if (strcmp("pair", action) == 0)
    client_pair(address);
  else if (strcmp("quit", action) == 0) {
    pair_check();
    client_quit_app(address);
  } else
    fprintf(stderr, "%s is not a valid action\n", action);
}
