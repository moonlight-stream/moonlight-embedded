/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2017 Sunguk Lee
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

#include <stdio.h>
#include <stdarg.h>
#include <psp2/rtc.h>

#include "debug.h"

void vita_debug_log(const char *s, ...) {
  if (!config.save_debug_log) {
    return;
  }

  char buffer[1024] = {0};

  SceDateTime time;
  sceRtcGetCurrentClock(&time, 0);

  snprintf(buffer, 26, "%04d%02d%02d %02d:%02d:%02d.%06d ",
           time.year, time.month, time.day,
           time.hour, time.minute, time.second,
           time.microsecond);

  va_list va;
  va_start(va, s);
  int len = vsnprintf(&buffer[25], 998, s, va);
  va_end(va);

  fprintf(config.log_file, buffer);
  if (buffer[len + 24] != '\n') {
      fprintf(config.log_file, "\n");
  }
  fflush(config.log_file);
}

