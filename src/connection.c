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
#include "global.h"
#include "config.h"
#include "power/vita.h"
#include "input/vita.h"
#include "video/vita.h"
#include "audio/vita.h"

#include <stdio.h>
#include <stdbool.h>

static int connection_status = LI_READY;

int connection_failed_stage = 0;
long connection_failed_stage_code = 0;

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
  connection_status = LI_CONNECTED;
  start_output();
}

void connection_connection_terminated() {
  stop_output();
  connection_status = LI_READY;
}

void connection_display_message(char *msg) {
  printf("%s\n", msg);
}

void connection_display_transient_message(char *msg) {
  printf("%s\n", msg);
}

void connection_reset() {
  connection_connection_terminated();
  connection_status = LI_READY;
}

void connection_minimize() {
  stop_output();
  connection_status = LI_MINIMIZED;
}

void connection_resume() {
  start_output();
  connection_status = LI_CONNECTED;
}

void connection_terminate() {
  connection_connection_terminated();
  LiStopConnection();
  connection_status = LI_READY;
}

void stage_failed(int stage, long code) {
  connection_failed_stage = stage;
  connection_failed_stage_code = code;
}

bool connection_is_ready() {
  return connection_status != LI_DISCONNECTED;
}

int connection_get_status() {
  return connection_status;
}

CONNECTION_LISTENER_CALLBACKS connection_callbacks = {
  .stageStarting = NULL,
  .stageComplete = NULL,
  .stageFailed = stage_failed,
  .connectionStarted = connection_connection_started,
  .connectionTerminated = connection_connection_terminated,
  .displayMessage = connection_display_message,
  .displayTransientMessage = connection_display_transient_message,
};
