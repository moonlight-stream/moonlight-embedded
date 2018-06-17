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

#include "audio/audio.h"

#include <ini.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>

#define MOONLIGHT_PATH "/moonlight"
#define USER_PATHS "."
#define DEFAULT_CONFIG_DIR "/.config"
#define DEFAULT_CACHE_DIR "/.cache"

#define write_config_string(fd, key, value) fprintf(fd, "%s = %s\n", key, value)
#define write_config_int(fd, key, value) fprintf(fd, "%s = %d\n", key, value)
#define write_config_bool(fd, key, value) fprintf(fd, "%s = %s\n", key, value?"true":"false");

bool inputAdded = false;

static struct option long_options[] = {
  {"720", no_argument, NULL, 'a'},
  {"1080", no_argument, NULL, 'b'},
  {"4k", no_argument, NULL, '0'},
  {"width", required_argument, NULL, 'c'},
  {"height", required_argument, NULL, 'd'},
  {"bitrate", required_argument, NULL, 'g'},
  {"packetsize", required_argument, NULL, 'h'},
  {"app", required_argument, NULL, 'i'},
  {"input", required_argument, NULL, 'j'},
  {"mapping", required_argument, NULL, 'k'},
  {"nosops", no_argument, NULL, 'l'},
  {"audio", required_argument, NULL, 'm'},
  {"localaudio", no_argument, NULL, 'n'},
  {"config", required_argument, NULL, 'o'},
  {"platform", required_argument, 0, 'p'},
  {"save", required_argument, NULL, 'q'},
  {"keydir", required_argument, NULL, 'r'},
  {"remote", no_argument, NULL, 's'},
  {"windowed", no_argument, NULL, 't'},
  {"surround", no_argument, NULL, 'u'},
  {"fps", required_argument, NULL, 'v'},
  {"codec", required_argument, NULL, 'x'},
  {"unsupported", no_argument, NULL, 'y'},
  {"verbose", no_argument, NULL, 'z'},
  {"debug", no_argument, NULL, 'Z'},
  {0, 0, 0, 0},
};

static int ini_handle(void *out, const char *section, const char *name, const char *value) {
#define HEX(v) strtol((v), NULL, 16)
#define INT(v) atoi((v))
#define BOOL(v) strcmp((v), "true") == 0
#define STR(v) strdup((v))

  PCONFIGURATION config = (PCONFIGURATION)out;

  if (strcmp(name, "address") == 0) {
    config->address = STR(value);
  } else if (strcmp(name, "width") == 0) {
    config->stream.width = INT(value);
  } else if (strcmp(name, "height") == 0) {
    config->stream.height = INT(value);
  } else if (strcmp(name, "bitrate") == 0) {
    config->stream.bitrate = INT(value);
  } else if (strcmp(name, "sops") == 0) {
    config->sops = BOOL(value);
  } else if (strcmp(name, "localaudio") == 0) {
    config->localaudio = BOOL(value);
  } else if (strcmp(name, "mapping") == 0) {
    config->mapping = STR(value);
  } else if (strcmp(name, "enable_remote_stream_optimization") == 0) {
    config->stream.streamingRemotely = INT(value);
  } else if (strcmp(name, "debug_level") == 0) {
    config->debug_level = INT(value);
  }

  return 1;
}

bool config_file_parse(char* filename, PCONFIGURATION config) {
  return ini_parse(filename, ini_handle, config);
}

void config_save(char* filename, PCONFIGURATION config) {
  FILE* fd = fopen(filename, "w");
  if (fd == NULL) {
    fprintf(stderr, "Can't open configuration file: %s\n", filename);
    exit(EXIT_FAILURE);
  }

  if (config->stream.width != 1280)
    write_config_int(fd, "width", config->stream.width);
  if (config->stream.height != 720)
    write_config_int(fd, "height", config->stream.height);
  if (config->stream.fps != 60)
    write_config_int(fd, "fps", config->stream.fps);
  if (config->stream.bitrate != -1)
    write_config_int(fd, "bitrate", config->stream.bitrate);
  if (config->stream.packetSize != 1024)
    write_config_int(fd, "packetsize", config->stream.packetSize);
  if (!config->sops)
    write_config_bool(fd, "sops", config->sops);
  if (config->localaudio)
    write_config_bool(fd, "localaudio", config->localaudio);

  if (strcmp(config->app, "Steam") != 0)
    write_config_string(fd, "app", config->app);

  fclose(fd);
}

void config_parse(char *filename, PCONFIGURATION config) {
  LiInitializeStreamConfiguration(&config->stream);

  config->stream.width = 1280;
  config->stream.height = 720;
  config->stream.fps = -1;
  config->stream.bitrate = -1;
  config->stream.packetSize = 1024;
  config->stream.streamingRemotely = 0;
  config->stream.audioConfiguration = AUDIO_CONFIGURATION_STEREO;
  config->stream.supportsHevc = false;

  config->debug_level = 0;
  config->platform = "switch";
  config->app = "Steam";
  config->action = NULL;
  config->address = NULL;
  config->config_file = NULL;
  config->audio_device = NULL;
  config->sops = true;
  config->localaudio = false;
  config->fullscreen = true;
  config->unsupported = false;
  config->codec = CODEC_UNSPECIFIED;

  config->inputsCount = 0;
  char* config_file = filename;

  if (config_file)
    config_file_parse(config_file, config);

  if (config->stream.fps == -1)
    config->stream.fps = config->stream.height >= 1080 ? 30 : 60;

  if (config->stream.bitrate == -1) {
    if (config->stream.height >= 1080 && config->stream.fps >= 60)
      config->stream.bitrate = 20000;
    else if (config->stream.height >= 1080 || config->stream.fps >= 60)
      config->stream.bitrate = 10000;
    else
      config->stream.bitrate = 5000;
  }
}
