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

#include <stdio.h>
#include <stdarg.h>
#include <signal.h>

pthread_t main_thread_id = 0;
bool connection_debug;

static void connection_stage_starting(int stage) {
//  printf("[*] Stage starting: %d\n", stage);
}
static void connection_stage_complete(int stage) {
//  printf("[*] Stage completed: %d\n", stage);
}
static void connection_stage_failed(int stage, long errorCode) {
//  printf("[*] Stage failed: %d, error: %ld\n", stage, errorCode);

}
static void connection_started(void) {
  printf("[*] Connection started\n");
}
static void connection_terminated() {
  printf("[*] Connection terminated\n");
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
