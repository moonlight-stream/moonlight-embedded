/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2017 Iwan Timmer
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

#include "util.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int blank_fb(char *path, bool clear) {
  int fd = open(path, O_RDWR);

  if(fd >= 0) {
    int ret = write(fd, clear ? "1" : "0", 1);
    if (ret < 0)
      fprintf(stderr, "Failed to clear framebuffer %s: %d\n", path, ret);

    close(fd);
    return 0;
  } else
    return -1;
}

uid_t getuid() {
  return 0;
}
