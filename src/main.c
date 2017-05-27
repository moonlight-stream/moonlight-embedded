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

#include "loop.h"
#include "client.h"
#include "connection.h"
#include "configuration.h"
#include "audio.h"
#include "video.h"
#include "discover.h"
#include "config.h"
#include "platform.h"
#include "sdl.h"

#include "input/mapping.h"
#include "input/evdev.h"
#include "input/udev.h"
#include "input/cec.h"
#include "input/sdlinput.h"

#include <Limelight.h>

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

static void applist(PSERVER_DATA server) {
  PAPP_LIST list = NULL;
  if (gs_applist(server, &list) != GS_OK) {
    fprintf(stderr, "Can't get app list\n");
    return;
  }

  for (int i = 1;list != NULL;i++) {
    printf("%d. %s\n", i, list->name);
    list = list->next;
  }
}

static int get_app_id(PSERVER_DATA server, const char *name) {
  PAPP_LIST list = NULL;
  if (gs_applist(server, &list) != GS_OK) {
    fprintf(stderr, "Can't get app list\n");
    return -1;
  }

  while (list != NULL) {
    if (strcmp(list->name, name) == 0)
      return list->id;

    list = list->next;
  }
  return -1;
}

static void stream(PSERVER_DATA server, PCONFIGURATION config, enum platform system) {
  int appId = get_app_id(server, config->app);
  if (appId<0) {
    fprintf(stderr, "Can't find app %s\n", config->app);
    exit(-1);
  }

  int ret = gs_start_app(server, &config->stream, appId, config->sops, config->localaudio);
  if (ret < 0) {
    if (ret == GS_NOT_SUPPORTED_4K)
      fprintf(stderr, "Server doesn't support 4K\n");
    else
      fprintf(stderr, "Errorcode starting app: %d\n", ret);
    exit(-1);
  }

  int drFlags = 0;
  if (config->fullscreen)
    drFlags |= DISPLAY_FULLSCREEN;

  if (config->forcehw)
    drFlags |= FORCE_HARDWARE_ACCELERATION;

  printf("Stream %d x %d, %d fps, %d kbps\n", config->stream.width, config->stream.height, config->stream.fps, config->stream.bitrate);
  LiStartConnection(&server->serverInfo, &config->stream, &connection_callbacks, platform_get_video(system), platform_get_audio(system), NULL, drFlags);

  if (IS_EMBEDDED(system)) {
    evdev_start();
    loop_main();
    evdev_stop();
  }
  #ifdef HAVE_SDL
  else if (system == SDL)
    sdl_loop();
  #endif

  LiStopConnection();
}

static void help() {
  printf("Usage: moonlight [action] (options) [host]\n");
  printf("       moonlight [configfile]\n");
  printf("\n Actions\n\n");
  printf("\tpair\t\t\tPair device with computer\n");
  printf("\tunpair\t\t\tUnpair device with computer\n");
  printf("\tstream\t\t\tStream computer to device\n");
  printf("\tlist\t\t\tList available games and applications\n");
  printf("\tquit\t\t\tQuit the application or game being streamed\n");
  printf("\thelp\t\t\tShow this help\n");
  printf("\n Global Options\n\n");
  printf("\t-config <config>\tLoad configuration file\n");
  printf("\t-save <config>\t\tSave configuration file\n");
  printf("\n Streaming options\n\n");
  printf("\t-720\t\t\tUse 1280x720 resolution [default]\n");
  printf("\t-1080\t\t\tUse 1920x1080 resolution\n");
  printf("\t-width <width>\t\tHorizontal resolution (default 1280)\n");
  printf("\t-height <height>\tVertical resolution (default 720)\n");
  printf("\t-30fps\t\t\tUse 30fps\n");
  printf("\t-60fps\t\t\tUse 60fps [default]\n");
  printf("\t-bitrate <bitrate>\tSpecify the bitrate in Kbps\n");
  printf("\t-packetsize <size>\tSpecify the maximum packetsize in bytes\n");
  printf("\t-hevc\t\t\tUse the high efficiency video coding (HEVC)\n");
  printf("\t-h264\t\t\tUse the advanced video coding (H264)\n");
  printf("\t-remote\t\t\tEnable remote optimizations\n");
  printf("\t-app <app>\t\tName of app to stream\n");
  printf("\t-nosops\t\t\tDon't allow GFE to modify game settings\n");
  printf("\t-localaudio\t\tPlay audio locally\n");
  printf("\t-surround\t\tStream 5.1 surround sound (requires GFE 2.7)\n");
  printf("\t-keydir <directory>\tLoad encryption keys from directory\n");
  printf("\t-mapping <file>\t\tUse <file> as gamepad mappings configuration file\n");
  #ifdef HAVE_SDL
  printf("\n Video options (SDL Only)\n\n");
  printf("\t-windowed\t\tDisplay screen in a window\n");
  #endif
  #ifdef HAVE_EMBEDDED
  printf("\n I/O options\n\n");
  printf("\t-input <device>\t\tUse <device> as input. Can be used multiple times\n");
  printf("\t-audio <device>\t\tUse <device> as audio output device\n");
  printf("\t-forcehw \t\tTry to use video hardware acceleration\n");
  #endif
  printf("\nUse Ctrl+Alt+Shift+Q to exit streaming session\n\n");
  exit(0);
}

static void pair_check(PSERVER_DATA server) {
  if (!server->paired) {
    fprintf(stderr, "You must pair with the PC first\n");
    exit(-1);
  }
}

int main(int argc, char* argv[]) {
  printf("Moonlight Embedded %d.%d.%d (%s)\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, COMPILE_OPTIONS);

  CONFIGURATION config;
  config_parse(argc, argv, &config);

  if (config.action == NULL || strcmp("help", config.action) == 0)
    help();
  
  enum platform system = platform_check(config.platform);
  if (system == 0) {
    fprintf(stderr, "Platform '%s' not found\n", config.platform);
    exit(-1);
  } else if (system == SDL && audio_device != NULL) {
    fprintf(stderr, "You can't select a audio device for SDL\n");
    exit(-1);
  }
  config.stream.supportsHevc = config.codec != CODEC_H264 && (config.codec == CODEC_HEVC || platform_supports_hevc(system));

  if (config.address == NULL) {
    config.address = malloc(MAX_ADDRESS_SIZE);
    if (config.address == NULL) {
      perror("Not enough memory");
      exit(-1);
    }
    config.address[0] = 0;
    printf("Searching for server...\n");
    gs_discover_server(config.address);
    if (config.address[0] == 0) {
      fprintf(stderr, "Autodiscovery failed. Specify an IP address next time.\n");
      exit(-1);
    }
  }
  
  char host_config_file[128];
  sprintf(host_config_file, "hosts/%s.conf", config.address);
  if (access(host_config_file, R_OK) != -1)
    config_file_parse(host_config_file, &config);

  SERVER_DATA server;
  printf("Connect to %s...\n", config.address);

  int ret;
  if ((ret = gs_init(&server, config.address, config.key_dir)) == GS_OUT_OF_MEMORY) {
    fprintf(stderr, "Not enough memory\n");
    exit(-1);
  } else if (ret == GS_INVALID) {
    fprintf(stderr, "Invalid data received from server: %s\n", config.address, gs_error);
    exit(-1);
  } else if (ret == GS_UNSUPPORTED_VERSION) {
    if (!config.unsupported_version) {
      fprintf(stderr, "Unsupported version: %s\n", gs_error);
      exit(-1);
    }
  } else if (ret != GS_OK) {
    fprintf(stderr, "Can't connect to server %s\n", config.address);
    exit(-1);
  }

  printf("NVIDIA %s, GFE %s (%s, %s)\n", server.gpuType, server.serverInfo.serverInfoGfeVersion, server.gsVersion, server.serverInfo.serverInfoAppVersion);

  if (strcmp("list", config.action) == 0) {
    pair_check(&server);
    applist(&server);
  } else if (strcmp("stream", config.action) == 0) {
    pair_check(&server);
    if (IS_EMBEDDED(system)) {
      struct mapping* mappings = mapping_load(config.mapping);

      for (int i=0;i<config.inputsCount;i++) {
        printf("Add input %s...\n", config.inputs[i]);
        evdev_create(config.inputs[i], mappings);
      }

      udev_init(!inputAdded, mappings);
      evdev_init();
      #ifdef HAVE_LIBCEC
      cec_init();
      #endif /* HAVE_LIBCEC */
    }
    #ifdef HAVE_SDL
    else if (system == SDL) {
      if (config.inputsCount > 0) {
        fprintf(stderr, "You can't select input devices as SDL will automatically use all available controllers\n");
        exit(-1);
      }

      sdl_init(config.stream.width, config.stream.height, config.fullscreen);
      sdlinput_init(config.mapping);
    }
    #endif

    stream(&server, &config, system);
  } else if (strcmp("pair", config.action) == 0) {
    char pin[5];
    sprintf(pin, "%d%d%d%d", (int)random() % 10, (int)random() % 10, (int)random() % 10, (int)random() % 10);
    printf("Please enter the following PIN on the target PC: %s\n", pin);
    if (gs_pair(&server, &pin[0]) != GS_OK) {
      fprintf(stderr, "Failed to pair to server: %s\n", gs_error);
    } else {
      printf("Succesfully paired\n");
    }
  } else if (strcmp("unpair", config.action) == 0) {
    if (gs_unpair(&server) != GS_OK) {
      fprintf(stderr, "Failed to unpair to server: %s\n", gs_error);
    } else {
      printf("Succesfully unpaired\n");
    }
  } else if (strcmp("quit", config.action) == 0) {
    pair_check(&server);
    gs_quit_app(&server);
  } else
    fprintf(stderr, "%s is not a valid action\n", config.action);
}
