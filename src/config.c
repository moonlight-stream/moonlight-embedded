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
#include "util.h"

#include "input/evdev.h"
#include "audio/audio.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <pwd.h>
#include <sys/types.h>

#define MOONLIGHT_PATH "/moonlight"
#define USER_PATHS "."
#define DEFAULT_CONFIG_DIR "/.config"
#define DEFAULT_CACHE_DIR "/.cache"

#define write_config_string(fd, key, value) fprintf(fd, "%s = %s\n", key, value)
#define write_config_int(fd, key, value) fprintf(fd, "%s = %d\n", key, value)
#define write_config_bool(fd, key, value) fprintf(fd, "%s = %s\n", key, value ? "true":"false")

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
  {"platform", required_argument, NULL, 'p'},
  {"save", required_argument, NULL, 'q'},
  {"keydir", required_argument, NULL, 'r'},
  {"remote", required_argument, NULL, 's'},
  {"windowed", no_argument, NULL, 't'},
  {"surround", required_argument, NULL, 'u'},
  {"fps", required_argument, NULL, 'v'},
  {"codec", required_argument, NULL, 'x'},
  {"nounsupported", no_argument, NULL, 'y'},
  {"quitappafter", no_argument, NULL, '1'},
  {"viewonly", no_argument, NULL, '2'},
  {"rotate", required_argument, NULL, '3'},
  {"verbose", no_argument, NULL, 'z'},
  {"debug", no_argument, NULL, 'Z'},
  {"nomouseemulation", no_argument, NULL, '4'},
  {"pin", required_argument, NULL, '5'},
  {0, 0, 0, 0},
};

char* get_path(char* name, char* extra_data_dirs) {
  const char *xdg_config_dir = getenv("XDG_CONFIG_DIR");
  const char *home_dir = getenv("HOME");

  if (access(name, R_OK) != -1) {
      return name;
  }

  if (!home_dir) {
    struct passwd *pw = getpwuid(getuid());
    home_dir = pw->pw_dir;
  }

  if (!extra_data_dirs)
    extra_data_dirs = "/usr/share:/usr/local/share";
  if (!xdg_config_dir)
    xdg_config_dir = home_dir;

  char *data_dirs = malloc(strlen(USER_PATHS) + 1 + strlen(xdg_config_dir) + 1 + strlen(home_dir) + 1 + strlen(DEFAULT_CONFIG_DIR) + 1 + strlen(extra_data_dirs) + 2);
  sprintf(data_dirs, USER_PATHS ":%s:%s/" DEFAULT_CONFIG_DIR ":%s/", xdg_config_dir, home_dir, extra_data_dirs);

  char *path = malloc(strlen(data_dirs)+strlen(MOONLIGHT_PATH)+strlen(name)+2);
  if (path == NULL) {
    fprintf(stderr, "Not enough memory\n");
    exit(-1);
  }

  char* data_dir = data_dirs;
  char* end;
  do {
    end = strstr(data_dir, ":");
    int length = end != NULL ? end - data_dir:strlen(data_dir);
    memcpy(path, data_dir, length);
    if (path[0] == '/')
      sprintf(path+length, MOONLIGHT_PATH "/%s", name);
    else
      sprintf(path+length, "/%s", name);

    if(access(path, R_OK) != -1) {
      free(data_dirs);
      return path;
    }

    data_dir = end + 1;
  } while (end != NULL);

  free(data_dirs);
  free(path);
  return NULL;
}

static void parse_argument(int c, char* value, PCONFIGURATION config) {
  switch (c) {
  case 'a':
    config->stream.width = 1280;
    config->stream.height = 720;
    break;
  case 'b':
    config->stream.width = 1920;
    config->stream.height = 1080;
    break;
  case '0':
    config->stream.width = 3840;
    config->stream.height = 2160;
    break;
  case 'c':
    config->stream.width = atoi(value);
    break;
  case 'd':
    config->stream.height = atoi(value);
    break;
  case 'g':
    config->stream.bitrate = atoi(value);
    break;
  case 'h':
    config->stream.packetSize = atoi(value);
    break;
  case 'i':
    config->app = value;
    break;
  case 'j':
    if (config->inputsCount >= MAX_INPUTS) {
      perror("Too many inputs specified");
      exit(-1);
    }
    config->inputs[config->inputsCount] = value;
    config->inputsCount++;
    inputAdded = true;
    break;
  case 'k':
    config->mapping = get_path(value, getenv("XDG_DATA_DIRS"));
    if (config->mapping == NULL) {
      fprintf(stderr, "Unable to open custom mapping file: %s\n", value);
      exit(-1);
    }
    break;
  case 'l':
    config->sops = false;
    break;
  case 'm':
    config->audio_device = value;
    break;
  case 'n':
    config->localaudio = true;
    break;
  case 'o':
    if (!config_file_parse(value, config))
      exit(EXIT_FAILURE);

    break;
  case 'p':
    config->platform = value;
    break;
  case 'q':
    config->config_file = value;
    break;
  case 'r':
    strcpy(config->key_dir, value);
    break;
  case 's':
    if (strcasecmp(value, "auto") == 0)
      config->stream.streamingRemotely = STREAM_CFG_AUTO;
    else if (strcasecmp(value, "true") == 0 || strcasecmp(value, "yes") == 0)
      config->stream.streamingRemotely = STREAM_CFG_REMOTE;
    else if (strcasecmp(value, "false") == 0 || strcasecmp(value, "no") == 0)
      config->stream.streamingRemotely = STREAM_CFG_LOCAL;
    break;

  case 't':
    config->fullscreen = false;
    break;
  case 'u':
    if (strcasecmp(value, "5.1") == 0)
      config->stream.audioConfiguration = AUDIO_CONFIGURATION_51_SURROUND;
    else if (strcasecmp(value, "7.1") == 0)
      config->stream.audioConfiguration = AUDIO_CONFIGURATION_71_SURROUND;
    break;
  case 'v':
    config->stream.fps = atoi(value);
    break;
  case 'x':
    if (strcasecmp(value, "auto") == 0)
      config->codec = CODEC_UNSPECIFIED;
    else if (strcasecmp(value, "h264") == 0)
      config->codec = CODEC_H264;
    else if (strcasecmp(value, "h265") == 0 || strcasecmp(value, "hevc") == 0)
      config->codec = CODEC_HEVC;
    break;
  case 'y':
    config->unsupported = false;
    break;
  case '1':
    config->quitappafter = true;
    break;
  case '2':
    config->viewonly = true;
    break;
  case '3':
    config->rotate = atoi(value);
    break;
  case 'z':
    config->debug_level = 1;
    break;
  case 'Z':
    config->debug_level = 2;
    break;
  case '4':
    config->mouse_emulation = false;
    break;
  case '5':
    config->pin = atoi(value);
    break;
  case 1:
    if (config->action == NULL)
      config->action = value;
    else if (config->address == NULL)
      config->address = value;
    else {
      perror("Too many options");
      exit(-1);
    }
  }
}

bool config_file_parse(char* filename, PCONFIGURATION config) {
  FILE* fd = fopen(filename, "r");
  if (fd == NULL) {
    fprintf(stderr, "Can't open configuration file: %s\n", filename);
    return false;
  }

  char *line = NULL;
  size_t len = 0;

  while (getline(&line, &len, fd) != -1) {
    char *key = NULL, *value = NULL;
    if (sscanf(line, "%ms = %m[^\n]", &key, &value) == 2) {
      if (strcmp(key, "address") == 0) {
        config->address = value;
      } else if (strcmp(key, "sops") == 0) {
        config->sops = strcmp("true", value) == 0;
      } else {
        for (int i=0;long_options[i].name != NULL;i++) {
          if (strcmp(long_options[i].name, key) == 0) {
            if (long_options[i].has_arg == required_argument)
              parse_argument(long_options[i].val, value, config);
            else if (strcmp("true", value) == 0)
              parse_argument(long_options[i].val, NULL, config);
          }
        }
      }
    }
  }
  return true;
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
  if (config->quitappafter)
    write_config_bool(fd, "quitappafter", config->quitappafter);
  if (config->viewonly)
    write_config_bool(fd, "viewonly", config->viewonly);
  if (config->rotate != 0)
    write_config_int(fd, "rotate", config->rotate);

  if (strcmp(config->app, "Steam") != 0)
    write_config_string(fd, "app", config->app);

  fclose(fd);
}

void config_parse(int argc, char* argv[], PCONFIGURATION config) {
  LiInitializeStreamConfiguration(&config->stream);

  config->stream.width = 1280;
  config->stream.height = 720;
  config->stream.fps = 60;
  config->stream.bitrate = -1;
  config->stream.packetSize = 1392;
  config->stream.streamingRemotely = STREAM_CFG_AUTO;
  config->stream.audioConfiguration = AUDIO_CONFIGURATION_STEREO;
  config->stream.supportsHevc = false;
  config->stream.encryptionFlags = ENCFLG_AUDIO;

#ifdef __arm__
  char cpuinfo[4096] = {};
  if (read_file("/proc/cpuinfo", cpuinfo, sizeof(cpuinfo) - 1) > 0) {
    // If this is a ARMv6 CPU (like the Pi 1), we'll assume it's not
    // powerful enough to handle audio encryption. The Pi 1 could
    // barely handle Opus decoding alone.
    if (strstr(cpuinfo, "ARMv6")) {
      config->stream.encryptionFlags = ENCFLG_NONE;
      printf("Disabling audio encryption on low performance CPU\n");
    }
  }
#endif

  config->debug_level = 0;
  config->platform = "auto";
  config->app = "Steam";
  config->action = NULL;
  config->address = NULL;
  config->config_file = NULL;
  config->audio_device = NULL;
  config->sops = true;
  config->localaudio = false;
  config->fullscreen = true;
  config->unsupported = true;
  config->quitappafter = false;
  config->viewonly = false;
  config->mouse_emulation = true;
  config->rotate = 0;
  config->codec = CODEC_UNSPECIFIED;
  config->pin = 0;

  config->inputsCount = 0;
  config->mapping = get_path("gamecontrollerdb.txt", getenv("XDG_DATA_DIRS"));
  config->key_dir[0] = 0;

  char* config_file = get_path("moonlight.conf", "/etc");
  if (config_file)
    config_file_parse(config_file, config);

  if (argc == 2 && access(argv[1], F_OK) == 0) {
    config->action = "stream";
    if (!config_file_parse(argv[1], config))
      exit(EXIT_FAILURE);

  } else {
    int option_index = 0;
    int c;
    while ((c = getopt_long_only(argc, argv, "-abc:d:efg:h:i:j:k:lm:no:p:q:r:s:tu:v:w:xy45:", long_options, &option_index)) != -1) {
      parse_argument(c, optarg, config);
    }
  }

  if (config->config_file != NULL)
    config_save(config->config_file, config);

  if (config->key_dir[0] == 0x0) {
    struct passwd *pw = getpwuid(getuid());
    const char *dir;
    if ((dir = getenv("XDG_CACHE_DIR")) != NULL)
      sprintf(config->key_dir, "%s" MOONLIGHT_PATH, dir);
    else if ((dir = getenv("HOME")) != NULL)
      sprintf(config->key_dir, "%s" DEFAULT_CACHE_DIR MOONLIGHT_PATH, dir);
    else
      sprintf(config->key_dir, "%s" DEFAULT_CACHE_DIR MOONLIGHT_PATH, pw->pw_dir);
  }

  if (config->stream.bitrate == -1) {
    if (config->stream.height >= 1080 && config->stream.fps >= 60)
      config->stream.bitrate = 20000;
    else if (config->stream.height >= 1080 || config->stream.fps >= 60)
      config->stream.bitrate = 10000;
    else
      config->stream.bitrate = 5000;
  }
}
