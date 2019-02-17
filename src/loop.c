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

#include "loop.h"

#include <stdlib.h>
#include <stdio.h>
#include <poll.h>
#include <string.h>

static struct pollfd* fds = NULL;
static FdHandler* fdHandlers = NULL;
static int numFds = 0;

void loop_add_fd(int fd, FdHandler handler, int events) {
  int fdindex = numFds;
  numFds++;

  if (fds == NULL) {
    fds = malloc(sizeof(struct pollfd));
    fdHandlers = malloc(sizeof(FdHandler*));
  } else {
    fds = realloc(fds, sizeof(struct pollfd)*numFds);
    fdHandlers = realloc(fdHandlers, sizeof(FdHandler*)*numFds);
  }

  if (fds == NULL || fdHandlers == NULL) {
    fprintf(stderr, "Not enough memory\n");
    exit(EXIT_FAILURE);
  }

  fds[fdindex].fd = fd;
  fds[fdindex].events = events;
  fdHandlers[fdindex] = handler;
}

void loop_remove_fd(int fd) {
  int fdindex;
  numFds--;

  for (int i=0; i<numFds; i++) {
    if (fds[i].fd = fd)
      fdindex = i;
      break;
  }

  if (fdindex != numFds && numFds > 0) {
    memcpy(&fds[fdindex], &fds[numFds], sizeof(struct pollfd));
    memcpy(&fdHandlers[fdindex], &fdHandlers[numFds], sizeof(FdHandler*));
  }
}

void loop_main() {
  while (poll(fds, numFds, -1))
    for (int i=0; i<numFds; i++)
      if (fds[i].revents > 0 && fdHandlers[i](fds[i].fd) == LOOP_RETURN)
        return;
}
