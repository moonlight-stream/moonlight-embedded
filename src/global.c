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

#include <stdarg.h>
#include <signal.h>
#include <pthread.h>

#include "config.h"
#include "debug.h"

// pthread_t main_thread_id;

void quit() {
  // pthread_kill(main_thread_id, SIGTERM);
}

void DEBUG_PRINT(const char *s, ...) {
  if (!config.save_debug_log) {
    return;
  }

  va_list va;
  char buffer[1024];

  va_start(va, s);
  vsprintf(buffer, s, va);
  va_end(va);

  fprintf(config.log_file, buffer);
  fflush(config.log_file);
}
