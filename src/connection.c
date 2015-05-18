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

#include <stdio.h>

void connection_stage_starting(int stage)
{
  printf("connection_stage_starting %d\n", stage);
}

void connection_stage_complete(int stage)
{
  printf("connection_stage_complete %d\n", stage);
}

void connection_stage_failed(int stage, long iets)
{
  printf("connection_stage_failed %d\n", stage);
}

void connection_connection_started()
{
  printf("connection_connection_started\n");
}

void connection_connection_terminated()
{
  quit();
  printf("connection_connection_terminated\n");
}

void connection_display_message(char *msg)
{
  printf("connection_display_message: %s\n", msg);
}

void connection_display_transient_message(char *msg)
{
  printf("connection_display_transient_message: %s\n", msg);
}

CONNECTION_LISTENER_CALLBACKS connection_callbacks = {
  .stageStarting = connection_stage_starting,
  .stageComplete = connection_stage_complete,
  .stageFailed = connection_stage_failed,
  .connectionStarted = connection_connection_started,
  .connectionTerminated = connection_connection_terminated,
  .displayMessage = connection_display_message,
  .displayTransientMessage = connection_display_transient_message,
};
