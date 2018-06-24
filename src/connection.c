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

#include "connection.h"
#include "configuration.h"
#include "config.h"
#include "platform.h"
#include "video/video.h"

#include <stdio.h>
#include <stdarg.h>
#include <signal.h>

pthread_t main_thread_id = 0;
bool connection_debug;

int pair_check(PSERVER_DATA server) {
  if (!server->paired) {
    fprintf(stderr, "You must pair with the PC first\n");
    return 0;
  }

  return 1;
}

size_t get_app_list(PSERVER_DATA server, PAPP_LIST *list) {
  if (gs_applist(server, list) != GS_OK) {
    fprintf(stderr, "Can't get app list\n");
    *list = NULL;
    return 0;
  }

  PAPP_LIST curr = *list;
  size_t count = 0;
  while (curr) {
    count++;
    curr = curr->next;
  }

  return count;
}

int get_app_id(PSERVER_DATA server, const char *name) {
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

int stream_start(PSERVER_DATA server, PCONFIGURATION config, int appId, enum platform system) {
  int gamepads = 0;
  int gamepad_mask = 0;
  for (int i = 0; i < gamepads && i < 4; i++)
    gamepad_mask = (gamepad_mask << 1) + 1;

  int ret = gs_start_app(server, &config->stream, appId, config->sops, config->localaudio, gamepad_mask);
  if (ret < 0) {
    if (ret == GS_NOT_SUPPORTED_4K)
      fprintf(stderr, "Server doesn't support 4K\n");
    else if (ret == GS_NOT_SUPPORTED_MODE)
      fprintf(stderr, "Server doesn't support %dx%d (%d fps) or try --unsupported option\n", config->stream.width, config->stream.height, config->stream.fps);
    else if (ret == GS_ERROR)
      fprintf(stderr, "Gamestream error: %s\n", gs_error);
    else
      fprintf(stderr, "Errorcode starting app: %d\n", ret);

    return -1;
  }

  int drFlags = 0;
  if (config->fullscreen)
    drFlags |= DISPLAY_FULLSCREEN;

  if (config->debug_level > 0) {
    printf("Stream %d x %d, %d fps, %d kbps\n", config->stream.width, config->stream.height, config->stream.fps, config->stream.bitrate);
    connection_debug = true;
  }

  platform_start(system);
  LiStartConnection(&server->serverInfo, &config->stream, &connection_callbacks, platform_get_video(system), platform_get_audio(system, config->audio_device), NULL, drFlags, config->audio_device, 0);

  return 0;
}

int stream_stop(enum platform system) {
  LiStopConnection();
  platform_stop(system);

  return 0;
}

// Moonlight connection callbacks
static void connection_stage_starting(int stage) {}
static void connection_stage_complete(int stage) {}
static void connection_stage_failed(int stage, long errorCode) {}
static void connection_started(void) {
  printf("[*] Connection started\n");
}
static void connection_terminated(long error) {
  perror("[*] Connection terminated");
}
static void connection_display_message(const char *msg) {
  printf("[*] %s\n", msg);
}
static void connection_display_transient_message(const char *msg) {
  printf("[*] %s\n", msg);
}
static void connection_log_message(const char* format, ...) {
  va_list arglist;
  va_start(arglist, format);
  vprintf(format, arglist);
  va_end(arglist);
}

CONNECTION_LISTENER_CALLBACKS connection_callbacks = {
  .stageStarting = connection_stage_starting,
  .stageComplete = connection_stage_complete,
  .stageFailed = connection_stage_failed,
  .connectionStarted = connection_started,
  .connectionTerminated = connection_terminated,
  .displayMessage = connection_display_message,
  .displayTransientMessage = connection_display_transient_message,
  .logMessage = connection_log_message,
};
