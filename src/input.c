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

#include "keyboard.h"

#include "libevdev/libevdev.h"
#include "limelight-common/Limelight.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>

struct input_device {
  struct libevdev *dev;
  int fd;
  __s32 mouseDeltaX, mouseDeltaY, mouseScroll;
};

static struct pollfd* fds = NULL;
static struct input_device* devices = NULL;
static int numDevices = 0;

void input_create(char* device) {
  int dev = numDevices;
  numDevices++;

  if (fds == NULL) {
    fds = malloc(sizeof(struct pollfd));
    devices = malloc(sizeof(struct input_device));
  } else {
    fds = realloc(fds, sizeof(struct pollfd)*numDevices);
    devices = realloc(devices, sizeof(struct input_device)*numDevices);
  }

  if (fds == NULL || devices == NULL) {
    fprintf(stderr, "Not enough memory\n");
    exit(EXIT_FAILURE);
  }

  devices[dev].fd = open(device, O_RDONLY|O_NONBLOCK);
  if (devices[dev].fd <= 0) {
    fprintf(stderr, "Failed to open device\n");
    exit(EXIT_FAILURE);
  }

  devices[dev].dev = libevdev_new();
  libevdev_set_fd(devices[dev].dev, devices[dev].fd);
  fds[dev].fd = devices[dev].fd;
  fds[dev].events = POLLIN;
}

static void handle_event(struct input_event *ev, struct input_device *dev) {
  switch (ev->type) {
  case EV_SYN:
    if (dev->mouseDeltaX != 0 || dev->mouseDeltaY != 0) {
      LiSendMouseMoveEvent(dev->mouseDeltaX, dev->mouseDeltaY);
      dev->mouseDeltaX = 0;
      dev->mouseDeltaY = 0;
    }
    if (dev->mouseScroll != 0) {
      LiSendScrollEvent(dev->mouseScroll);
      dev->mouseScroll = 0;
    }
    break;
  case EV_KEY:
    if (ev->code < sizeof(keyCodes)/sizeof(keyCodes[0])) {
      short code = 0x80 << 8 | keyCodes[ev->code];
      LiSendKeyboardEvent(code, ev->value?KEY_ACTION_DOWN:KEY_ACTION_UP, 0);
    } else {
      int mouseCode = 0;
      switch (ev->code) {
      case BTN_LEFT:
        mouseCode = BUTTON_LEFT;
        break;
      case BTN_MIDDLE:
        mouseCode = BUTTON_MIDDLE;
        break;
      case BTN_RIGHT:
        mouseCode = BUTTON_RIGHT;
        break;
      }

      if (mouseCode > 0) {
        LiSendMouseButtonEvent(ev->value?BUTTON_ACTION_PRESS:BUTTON_ACTION_RELEASE, mouseCode);
      }
    }
    break;
  case EV_REL:
    switch (ev->code) {
      case REL_X:
        dev->mouseDeltaX = ev->value;
        break;
      case REL_Y:
        dev->mouseDeltaY = ev->value;
        break;
      case REL_Z:
        dev->mouseScroll = ev->value;
        break;
    }
    break;
  }
}

void input_loop() {
  while (poll(fds, numDevices, -1)) {
    for (int i=0;i<numDevices;i++) {
      if (fds[i].revents > 0) {
        int rc;
        struct input_event ev;
        while ((rc = libevdev_next_event(devices[i].dev, LIBEVDEV_READ_FLAG_NORMAL, &ev)) >= 0) {
          if (rc == LIBEVDEV_READ_STATUS_SYNC)
            fprintf(stderr, "Error: cannot keep up\n");
          else if (rc == LIBEVDEV_READ_STATUS_SUCCESS);
            handle_event(&ev, &devices[i]);
        }
        if (rc != -EAGAIN && rc < 0) {
          fprintf(stderr, "Error: %s\n", strerror(-rc));
          exit(EXIT_FAILURE);
        }
      }
    }
  }
}
