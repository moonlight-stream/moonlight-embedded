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

#include "connection.h"
#include "config.h"
#include "power/vita.h"
#include "input/vita.h"
#include "video/vita.h"
#include "audio/vita.h"

#include <stdio.h>
#include <stdbool.h>

#include "debug.h"

static int connection_status = LI_DISCONNECTED;

int connection_failed_stage = 0;
long connection_failed_stage_code = 0;

void pause_output() {
  vitainput_stop();
  vitavideo_stop();
  vitaaudio_stop();
}

void stop_output() {
  vitainput_stop();
  vitapower_stop();
  vitavideo_stop();
  vitaaudio_stop();
}

void start_output() {
  vitainput_start();
  vitapower_start();
  vitavideo_start();
  vitaaudio_start();
}

void connection_connection_started() {
  if (connection_status != LI_PAIRED) {
    DEBUG_PRINT("connection_connection_started error: %d\n", connection_status);
    return;
  }
  DEBUG_PRINT("connection started\n");
  connection_status = LI_CONNECTED;
  start_output();
}

void connection_connection_terminated() {
  if (connection_status != LI_PAIRED && connection_status != LI_CONNECTED &&
      connection_status != LI_MINIMIZED) {
    DEBUG_PRINT("connection_connection_terminated error: %d\n", connection_status);
  }

  LiStopConnection();

  if (connection_status == LI_CONNECTED) {
    stop_output();
  }
  DEBUG_PRINT("connection terminated\n");
  connection_status = LI_DISCONNECTED;
}

void connection_display_message(const char *msg) {
  DEBUG_PRINT("display_message: %s\n", msg);
}

void connection_display_transient_message(const char *msg) {
  DEBUG_PRINT("display_transient_message: %s\n", msg);
}

int connection_reset() {
  if (connection_status != LI_DISCONNECTED) {
    DEBUG_PRINT("connection_reset error: %d\n", connection_status);
    return -1;
  }
  connection_status = LI_READY;
  return 0;
}

int connection_paired() {
  if (connection_status != LI_READY && connection_status != LI_PAIRED &&
      connection_status != LI_CONNECTED) {
    DEBUG_PRINT("connection_paired error: %d\n", connection_status);
    return -1;
  }
  connection_status = LI_PAIRED;
  return 0;
}

int connection_minimize() {
  if (connection_status != LI_CONNECTED) {
    DEBUG_PRINT("connection_minimize error: %d\n", connection_status);
    return -1;
  }
  pause_output();
  connection_status = LI_MINIMIZED;
  return 0;
}

int connection_resume() {
  if (connection_status != LI_MINIMIZED) {
    DEBUG_PRINT("connection_resume error: %d\n", connection_status);
    return -1;
  }
  start_output();
  connection_status = LI_CONNECTED;
  return 0;
}

int connection_terminate() {
  if (connection_status != LI_PAIRED && connection_status != LI_CONNECTED &&
      connection_status != LI_MINIMIZED) {
    DEBUG_PRINT("connection_terminate error: %d\n", connection_status);
    return -1;
  }
  connection_connection_terminated();
  return 0;
}

void connection_stage_starting(int stage) {
  DEBUG_PRINT("connection_stage_starting - stage: %d\n", stage);
}
void connection_stage_complate(int stage) {
  DEBUG_PRINT("connection_stage_complate - stage: %d\n", stage);
}

void connection_stage_failed(int stage, long code) {
  connection_failed_stage = stage;
  connection_failed_stage_code = code;
  DEBUG_PRINT("connection_stage_failed - stage: %d, %d\n", stage, code);
}

bool connection_is_ready() {
  return connection_status != LI_DISCONNECTED;
}

int connection_get_status() {
  return connection_status;
}

CONNECTION_LISTENER_CALLBACKS connection_callbacks = {
  .stageStarting = connection_stage_starting,
  .stageComplete = connection_stage_complate,
  .stageFailed = connection_stage_failed,
  .connectionStarted = connection_connection_started,
  .connectionTerminated = connection_connection_terminated,
  .displayMessage = connection_display_message,
  .displayTransientMessage = connection_display_transient_message,
};
