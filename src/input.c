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

#include <libudev.h>
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
  int fdindex;
  __s32 mouseDeltaX, mouseDeltaY, mouseScroll;
  short controllerId;
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
static int numDevices = 0, numFds = 0;
static int assignedControllerIds = 0;

static short* currentKey;
static short* currentAbs;
static bool* currentReverse;

static bool autoadd;
static char* defaultMapfile;
static struct udev *udev;
static struct udev_monitor *udev_mon;
static int udev_fdindex;

static void input_init_parms(struct input_device *dev, struct input_abs_parms *parms, int code) {
  parms->flat = libevdev_get_abs_flat(dev->dev, code);
  parms->min = libevdev_get_abs_minimum(dev->dev, code);
  parms->max = libevdev_get_abs_maximum(dev->dev, code);
  parms->avg = (parms->min+parms->max)/2;
  parms->range = parms->max - parms->avg;
  parms->diff = parms->max - parms->min;
}

void input_create(char* device, char* mapFile) {
  int fd = open(device, O_RDONLY|O_NONBLOCK);
  if (fd <= 0) {
    fprintf(stderr, "Failed to open device %s\n", device);
    fflush(stderr);
    return;
  }

  int dev = numDevices;
  int fdindex = numFds;
  numDevices++;
  numFds++;

  if (fds == NULL) {
    fds = malloc(sizeof(struct pollfd));
    devices = malloc(sizeof(struct input_device));
  } else {
    fds = realloc(fds, sizeof(struct pollfd)*numFds);
    devices = realloc(devices, sizeof(struct input_device)*numDevices);
  }

  if (fds == NULL || devices == NULL) {
    fprintf(stderr, "Not enough memory\n");
    exit(EXIT_FAILURE);
  }

  devices[dev].fd = fd;
  devices[dev].dev = libevdev_new();
  libevdev_set_fd(devices[dev].dev, devices[dev].fd);
  devices[dev].fdindex = fdindex;
  fds[fdindex].fd = devices[dev].fd;
  fds[fdindex].events = POLLIN;

  if (mapFile != NULL)
    mapping_load(mapFile, &(devices[dev].map));

  devices[dev].controllerId = -1;
  input_init_parms(&devices[dev], &(devices[dev].xParms), devices[dev].map.abs_x);
  input_init_parms(&devices[dev], &(devices[dev].yParms), devices[dev].map.abs_y);
  input_init_parms(&devices[dev], &(devices[dev].zParms), devices[dev].map.abs_z);
  input_init_parms(&devices[dev], &(devices[dev].rxParms), devices[dev].map.abs_rx);
  input_init_parms(&devices[dev], &(devices[dev].ryParms), devices[dev].map.abs_ry);
  input_init_parms(&devices[dev], &(devices[dev].rzParms), devices[dev].map.abs_rz);
  input_init_parms(&devices[dev], &(devices[dev].dpadxParms), devices[dev].map.abs_dpad_x);
  input_init_parms(&devices[dev], &(devices[dev].dpadyParms), devices[dev].map.abs_dpad_y);
}

void input_init(char* mapfile) {
  udev = udev_new();
  if (!udev) {
    fprintf(stderr, "Can't create udev\n");
    exit(1);
  }

  if (autoadd = numDevices == 0) {
    struct udev_enumerate *enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "input");
    udev_enumerate_scan_devices(enumerate);
    struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);

    struct udev_list_entry *dev_list_entry;
    udev_list_entry_foreach(dev_list_entry, devices) {
      const char *path = udev_list_entry_get_name(dev_list_entry);
      struct udev_device *dev = udev_device_new_from_syspath(udev, path);
      const char *devnode = udev_device_get_devnode(dev);
      int id;
      if (devnode != NULL && sscanf(devnode, "/dev/input/event%d", &id) == 1) {
        input_create(devnode, mapfile);
      }
      udev_device_unref(dev);
    }

    udev_enumerate_unref(enumerate);
  }

  udev_mon = udev_monitor_new_from_netlink(udev, "udev");
  udev_monitor_filter_add_match_subsystem_devtype(udev_mon, "input", NULL);
  udev_monitor_enable_receiving(udev_mon);

  int udev_fdindex = numFds;
  numFds++;

  if (fds == NULL)
    fds = malloc(sizeof(struct pollfd));
  else
    fds = realloc(fds, sizeof(struct pollfd)*numFds);

  if (fds == NULL) {
    fprintf(stderr, "Not enough memory\n");
    exit(EXIT_FAILURE);
  }

  defaultMapfile = mapfile;
  fds[udev_fdindex].fd = udev_monitor_get_fd(udev_mon);
  fds[udev_fdindex].events = POLLIN;
}

void input_destroy() {
  udev_unref(udev);
}

static short input_convert_value(struct input_event *ev, struct input_device *dev, struct input_abs_parms *parms, bool reverse) {
  if (abs(ev->value - parms->avg) < parms->flat)
    return 0;
  else if (ev->value > parms->max)
    return reverse?SHRT_MIN:SHRT_MAX;
  else if (ev->value < parms->min)
    return reverse?SHRT_MAX:SHRT_MIN;
  else {
    int value = ev->value + (ev->value<parms->avg?parms->flat:-parms->flat);
    return (value-parms->avg) * SHRT_MAX / ((reverse?-parms->range-1:parms->range) - parms->flat);
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

static int input_convert_value_direction(struct input_event *ev, struct input_device *dev, struct input_abs_parms *parms, bool reverse) {
  if (ev->value > (parms->avg+parms->range/4))
    return reverse?-1:1;
  else if (ev->value < (parms->avg-parms->range/4))
    return reverse?1:-1;
  else
    return 0;
}

static bool input_handle_event(struct input_event *ev, struct input_device *dev) {
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
      if (dev->controllerId < 0) {
        for (int i = 0; i < 4; i++) {
          if ((assignedControllerIds & (1 << i)) == 0) {
            assignedControllerIds |= (1 << i);
            dev->controllerId = i;
            break;
          }
        }
        //Use id 0 when too many gamepads are connected
        if (dev->controllerId < 0)
          dev->controllerId = 0;
      }
      LiSendMultiControllerEvent(dev->controllerId, dev->buttonFlags, dev->leftTrigger, dev->rightTrigger, dev->leftStickX, dev->leftStickY, dev->rightStickX, dev->rightStickY);
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
      dev->leftStickX = input_convert_value(ev, dev, &dev->xParms, dev->map.reverse_x);
    else if (ev->code == dev->map.abs_y)
      dev->leftStickY = input_convert_value(ev, dev, &dev->yParms, dev->map.reverse_y);
    else if (ev->code == dev->map.abs_rx)
      dev->rightStickX = input_convert_value(ev, dev, &dev->rxParms, dev->map.reverse_rx);
    else if (ev->code == dev->map.abs_ry)
      dev->rightStickY = input_convert_value(ev, dev, &dev->ryParms, dev->map.reverse_ry);
    else if (ev->code == dev->map.abs_z)
      dev->leftTrigger = input_convert_value_byte(ev, dev, &dev->zParms);
    else if (ev->code == dev->map.abs_rz)
      dev->rightTrigger = input_convert_value_byte(ev, dev, &dev->rzParms);
    else if (ev->code == dev->map.abs_dpad_x) {
      int dir = input_convert_value_direction(ev, dev, &dev->dpadxParms, dev->map.reverse_dpad_x);
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
      int dir = input_convert_value_direction(ev, dev, &dev->dpadyParms, dev->map.reverse_dpad_y);
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
  return true;
}

static bool input_handle_mapping_event(struct input_event *ev, struct input_device *dev) {
  switch (ev->type) {
  case EV_KEY:
    if (currentKey != NULL) {
      if (ev->value)
        *currentKey = ev->code;
      else if (ev->code == *currentKey)
        return false;
    }
    break;
  case EV_ABS:
    if (currentAbs != NULL) {
      struct input_abs_parms parms;
      input_init_parms(dev, &parms, ev->code);

      if (ev->value > parms.avg + parms.range/2) {
        *currentAbs = ev->code;
        *currentReverse = false;
      } else if (ev->value < parms.avg - parms.range/2) {
        *currentAbs = ev->code;
        *currentReverse = true;
      } else if (ev->code == *currentAbs)
        return false;
    }
    break;
  }
  return true;
}

static void input_poll(bool (*handler) (struct input_event*, struct input_device*)) {
  while (poll(fds, numFds, -1)) {
    if (fds[udev_fdindex].revents > 0) {
      struct udev_device *dev = udev_monitor_receive_device(udev_mon);
      const char *action = udev_device_get_action(dev);
      if (action != NULL) {
        if (autoadd && strcmp("add", action) == 0) {
          const char *devnode = udev_device_get_devnode(dev);
          int id;
          if (devnode != NULL && sscanf(devnode, "/dev/input/event%d", &id) == 1) {
            input_create(devnode, defaultMapfile);
          }
        }
        udev_device_unref(dev);
      }
    }
    for (int i=0;i<numDevices;i++) {
      if (fds[devices[i].fdindex].revents > 0) {
        int rc;
        struct input_event ev;
        while ((rc = libevdev_next_event(devices[i].dev, LIBEVDEV_READ_FLAG_NORMAL, &ev)) >= 0) {
          if (rc == LIBEVDEV_READ_STATUS_SYNC)
            fprintf(stderr, "Error: cannot keep up\n");
          else if (rc == LIBEVDEV_READ_STATUS_SUCCESS) {
            if (!handler(&ev, &devices[i]))
              return;
          }
        }
        if (rc != -EAGAIN && rc < 0) {
          fprintf(stderr, "Error: %s\n", strerror(-rc));
          exit(EXIT_FAILURE);
        }
      }
    }
  }
}

static void input_map_key(char* keyName, short* key) {
  printf("Press %s\n", keyName);
  currentKey = key;
  currentAbs = NULL;
  *key = -1;
  input_poll(input_handle_mapping_event);
}

static void input_map_abs(char* keyName, short* abs, bool* reverse) {
  printf("%s\n", keyName);
  currentKey = NULL;
  currentAbs = abs;
  currentReverse = reverse;
  *abs = -1;
  input_poll(input_handle_mapping_event);
}

static void input_map_abskey(char* keyName, short* key, short* abs, bool* reverse) {
  printf("%s\n", keyName);
  currentKey = key;
  currentAbs = abs;
  currentReverse = reverse;
  *key = -1;
  *abs = -1;
  *currentReverse = false;
  input_poll(input_handle_mapping_event);
}

void input_map(char* fileName) {
  struct mapping map;

  input_map_abs("Left Stick Right", &(map.abs_x), &(map.reverse_x));
  input_map_abs("Left Stick Down", &(map.abs_y), &(map.reverse_y));
  input_map_key("Left Stick Button", &(map.btn_thumbl));

  input_map_abs("Right Stick Right", &(map.abs_rx), &(map.reverse_rx));
  input_map_abs("Right Stick Down", &(map.abs_ry), &(map.reverse_ry));
  input_map_key("Right Stick Button", &(map.btn_thumbr));

  input_map_abskey("D-Pad Right", &(map.btn_dpad_right), &(map.abs_dpad_x), &(map.reverse_dpad_x));
  if (map.btn_dpad_right >= 0)
    input_map_key("D-Pad Left", &(map.btn_dpad_left));
  else
    map.btn_dpad_left = -1;

  input_map_abskey("D-Pad Down", &(map.btn_dpad_down), &(map.abs_dpad_y), &(map.reverse_dpad_y));
  if (map.btn_dpad_down >= 0)
    input_map_key("D-Pad Up", &(map.btn_dpad_up));
  else
    map.btn_dpad_up = -1;

  input_map_key("Button X (1)", &(map.btn_east));
  input_map_key("Button A (2)", &(map.btn_south));
  input_map_key("Button B (3)", &(map.btn_west));
  input_map_key("Button Y (4)", &(map.btn_north));
  input_map_key("Back Button", &(map.btn_select));
  input_map_key("Start Button", &(map.btn_start));
  input_map_key("Special Button", &(map.btn_mode));

  bool ignored;
  input_map_abskey("Left Trigger", &(map.btn_tl), &(map.abs_z), &ignored);
  input_map_abskey("Right Trigger", &(map.btn_tr), &(map.abs_rz), &ignored);

  input_map_key("Left Bumper", &(map.btn_tl2));
  input_map_key("Right Bumper", &(map.btn_tr2));
  mapping_save(fileName, &map);
}

void input_loop() {
  input_poll(input_handle_event);
}
