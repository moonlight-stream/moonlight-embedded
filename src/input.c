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
#include "mapping.h"

#include "libevdev/libevdev.h"
#include "limelight-common/Limelight.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <limits.h>

struct input_abs_parms {
  int min, max;
  int flat;
  int avg;
  int range, diff;
};

struct input_device {
  struct libevdev *dev;
  struct mapping map;
  int fd;
  __s32 mouseDeltaX, mouseDeltaY, mouseScroll;
  int buttonFlags;
  short leftTrigger, rightTrigger;
  short leftStickX, leftStickY;
  short rightStickX, rightStickY;
  bool gamepadModified;
  struct input_abs_parms xParms, yParms, rxParms, ryParms, zParms, rzParms;
  struct input_abs_parms dpadxParms, dpadyParms;
};

static struct pollfd* fds = NULL;
static struct input_device* devices = NULL;
static int numDevices = 0;

static void input_init_parms(struct input_device *dev, struct input_abs_parms *parms, int code) {
  parms->flat = libevdev_get_abs_flat(dev->dev, code);
  parms->min = libevdev_get_abs_minimum(dev->dev, code);
  parms->max = libevdev_get_abs_maximum(dev->dev, code);
  parms->avg = (parms->min+parms->max)/2;
  parms->range = parms->max - parms->avg;
  parms->diff = parms->max - parms->min;
}

void input_create(char* device, char* mapFile) {
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

  if (mapFile != NULL)
    mapping_load(mapFile, &(devices[dev].map));

  input_init_parms(&devices[dev], &(devices[dev].xParms), devices[dev].map.abs_x);
  input_init_parms(&devices[dev], &(devices[dev].yParms), devices[dev].map.abs_y);
  input_init_parms(&devices[dev], &(devices[dev].zParms), devices[dev].map.abs_z);
  input_init_parms(&devices[dev], &(devices[dev].rxParms), devices[dev].map.abs_rx);
  input_init_parms(&devices[dev], &(devices[dev].ryParms), devices[dev].map.abs_ry);
  input_init_parms(&devices[dev], &(devices[dev].rzParms), devices[dev].map.abs_rz);
  input_init_parms(&devices[dev], &(devices[dev].dpadxParms), devices[dev].map.abs_dpad_x);
  input_init_parms(&devices[dev], &(devices[dev].dpadyParms), devices[dev].map.abs_dpad_y);
}

static short input_convert_value(struct input_event *ev, struct input_device *dev, struct input_abs_parms *parms) {
  if (abs(ev->value) < parms->flat)
    return 0;
  else if (ev->value > parms->max)
    return SHRT_MAX;
  else if (ev->value < parms->min)
    return SHRT_MIN;
  else {
    int value = ev->value + (ev->value<parms->avg?parms->flat:-parms->flat);
    return (value-parms->avg) * SHRT_MAX / (parms->range - parms->flat);
  }
}

static char input_convert_value_byte(struct input_event *ev, struct input_device *dev, struct input_abs_parms *parms) {
  if (abs(ev->value-parms->min)<parms->flat)
    return 0;
  else if (ev->value>parms->max)
    return UCHAR_MAX;
  else {
    int value = ev->value + parms->flat;
    return (value-parms->min) * UCHAR_MAX / (parms->diff-parms->flat);
  }
}

static int input_convert_value_direction(struct input_event *ev, struct input_device *dev, struct input_abs_parms *parms) {
  if (ev->value > (parms->avg+parms->range/4))
    return 1;
  else if (ev->value < (parms->avg-parms->range/4))
    return -1;
  else
    return 0;
}

static void input_handle_event(struct input_event *ev, struct input_device *dev) {
  bool gamepadModified = false;

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
    if (dev->gamepadModified) {
      LiSendControllerEvent(dev->buttonFlags, dev->leftTrigger, dev->rightTrigger, dev->leftStickX, dev->leftStickY, dev->rightStickX, dev->rightStickY);
      dev->gamepadModified = false;
    }
    break;
  case EV_KEY:
    if (ev->code < sizeof(keyCodes)/sizeof(keyCodes[0])) {
      short code = 0x80 << 8 | keyCodes[ev->code];
      LiSendKeyboardEvent(code, ev->value?KEY_ACTION_DOWN:KEY_ACTION_UP, 0);
    } else {
      int mouseCode = 0;
      short gamepadCode = 0;
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
      default:
        if (ev->code == dev->map.btn_south)
          gamepadCode = A_FLAG;
        else if (ev->code == dev->map.btn_west)
          gamepadCode = X_FLAG;
        else if (ev->code == dev->map.btn_north)
          gamepadCode = Y_FLAG;
        else if (ev->code == dev->map.btn_east)
          gamepadCode = B_FLAG;
        else if (ev->code == dev->map.btn_dpad_up)
          gamepadCode = UP_FLAG;
        else if (ev->code == dev->map.btn_dpad_down)
          gamepadCode = DOWN_FLAG;
        else if (ev->code == dev->map.btn_dpad_right)
          gamepadCode = RIGHT_FLAG;
        else if (ev->code == dev->map.btn_dpad_left)
          gamepadCode = LEFT_FLAG;
        else if (ev->code == dev->map.btn_thumbl)
          gamepadCode = LS_CLK_FLAG;
        else if (ev->code == dev->map.btn_thumbr)
          gamepadCode = RS_CLK_FLAG;
        else if (ev->code == dev->map.btn_tl)
          gamepadCode = LB_FLAG;
        else if (ev->code == dev->map.btn_tr)
          gamepadCode = RB_FLAG;
        else if (ev->code == dev->map.btn_start)
          gamepadCode = PLAY_FLAG;
        else if (ev->code == dev->map.btn_select)
          gamepadCode = BACK_FLAG;
        else if (ev->code == dev->map.btn_mode)
          gamepadCode = SPECIAL_FLAG;
      }

      if (mouseCode > 0) {
        LiSendMouseButtonEvent(ev->value?BUTTON_ACTION_PRESS:BUTTON_ACTION_RELEASE, mouseCode);
      } else {
        gamepadModified = true;

        if (gamepadCode > 0) {
          if (ev->value)
            dev->buttonFlags |= gamepadCode;
          else
            dev->buttonFlags &= ~gamepadCode;
        } else if (ev->code == dev->map.btn_tl2)
          dev->leftTrigger = ev->value?USHRT_MAX:0;
        else if (ev->code == dev->map.btn_tr2)
          dev->rightTrigger = ev->value?USHRT_MAX:0;
        else
          gamepadModified = false;
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
  case EV_ABS:
    gamepadModified = true;
    if (ev->code == dev->map.abs_x)
      dev->leftStickX = input_convert_value(ev, dev, &dev->xParms);
    else if (ev->code == dev->map.abs_y)
      dev->leftStickY = input_convert_value(ev, dev, &dev->yParms);
    else if (ev->code == dev->map.abs_rx)
      dev->rightStickX = input_convert_value(ev, dev, &dev->rxParms);
    else if (ev->code == dev->map.abs_ry)
      dev->rightStickY = input_convert_value(ev, dev, &dev->ryParms);
    else if (ev->code == dev->map.abs_z)
      dev->leftTrigger = input_convert_value_byte(ev, dev, &dev->zParms);
    else if (ev->code == dev->map.abs_rz)
      dev->rightTrigger = input_convert_value_byte(ev, dev, &dev->rzParms);
    else if (ev->code == dev->map.abs_dpad_x) {
      int dir = input_convert_value_direction(ev, dev, &dev->dpadxParms);
      if (dir == 1) {
        dev->buttonFlags |= RIGHT_FLAG;
        dev->buttonFlags &= ~LEFT_FLAG;
      } else if (dir == -1) {
        dev->buttonFlags &= ~RIGHT_FLAG;
        dev->buttonFlags &= ~LEFT_FLAG;
      } else {
        dev->buttonFlags &= ~RIGHT_FLAG;
        dev->buttonFlags |= LEFT_FLAG;
      }
    } else if (ev->code == dev->map.abs_dpad_y) {
      int dir = input_convert_value_direction(ev, dev, &dev->dpadyParms);
      if (dir == 1) {
        dev->buttonFlags |= UP_FLAG;
        dev->buttonFlags &= ~DOWN_FLAG;
      } else if (dir == -1) {
        dev->buttonFlags &= ~UP_FLAG;
        dev->buttonFlags &= ~DOWN_FLAG;
      } else {
        dev->buttonFlags &= ~DOWN_FLAG;
        dev->buttonFlags |= UP_FLAG;
      }
    } else
      gamepadModified = false;

    break;
  }

  dev->gamepadModified |= gamepadModified;
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
            input_handle_event(&ev, &devices[i]);
        }
        if (rc != -EAGAIN && rc < 0) {
          fprintf(stderr, "Error: %s\n", strerror(-rc));
          exit(EXIT_FAILURE);
        }
      }
    }
  }
}
