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

#include "../loop.h"
#include "../global.h"

#include "keyboard.h"
#include "mapping.h"

#include "libevdev/libevdev.h"
#include <Limelight.h>

#include <libudev.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <poll.h>
#include <limits.h>
#include <unistd.h>
#include <pthread.h>

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
  char modifiers;
  __s32 mouseDeltaX, mouseDeltaY, mouseScroll;
  short controllerId;
  int buttonFlags;
  char leftTrigger, rightTrigger;
  short leftStickX, leftStickY;
  short rightStickX, rightStickY;
  bool gamepadModified;
  struct input_abs_parms xParms, yParms, rxParms, ryParms, zParms, rzParms;
  struct input_abs_parms dpadxParms, dpadyParms;
};

static struct input_device* devices = NULL;
static int numDevices = 0;
static int assignedControllerIds = 0;

static short* currentKey;
static short* currentAbs;
static bool* currentReverse;

static bool grabbingDevices;

#define QUIT_MODIFIERS (MODIFIER_SHIFT|MODIFIER_ALT|MODIFIER_CTRL)
#define QUIT_KEY KEY_Q

static bool (*handler) (struct input_event*, struct input_device*);

static void evdev_init_parms(struct input_device *dev, struct input_abs_parms *parms, int code) {
  parms->flat = libevdev_get_abs_flat(dev->dev, code);
  parms->min = libevdev_get_abs_minimum(dev->dev, code);
  parms->max = libevdev_get_abs_maximum(dev->dev, code);
  parms->avg = (parms->min+parms->max)/2;
  parms->range = parms->max - parms->avg;
  parms->diff = parms->max - parms->min;
}

static void evdev_remove(int devindex) {
  numDevices--;

  if (devices[devindex].controllerId >= 0)
    assignedControllerIds &= ~(1 << devices[devindex].controllerId);

  if (devindex != numDevices && numDevices > 0)
    memcpy(&devices[devindex], &devices[numDevices], sizeof(struct input_device));

  fprintf(stderr, "Removed input device\n");
}

static short evdev_convert_value(struct input_event *ev, struct input_device *dev, struct input_abs_parms *parms, bool reverse) {
  if (abs(ev->value - parms->avg) < parms->flat)
    return 0;
  else if (ev->value > parms->max)
    return reverse?SHRT_MIN:SHRT_MAX;
  else if (ev->value < parms->min)
    return reverse?SHRT_MAX:SHRT_MIN;
  else if (reverse)
    return (long long)(parms->max - (ev->value<parms->avg?parms->flat*2:0) - ev->value) * (SHRT_MAX-SHRT_MIN) / (parms->max-parms->min-parms->flat*2) + SHRT_MIN;
  else
    return (long long)(ev->value - (ev->value>parms->avg?parms->flat*2:0) - parms->min) * (SHRT_MAX-SHRT_MIN) / (parms->max-parms->min-parms->flat*2) + SHRT_MIN;
}

static char evdev_convert_value_byte(struct input_event *ev, struct input_device *dev, struct input_abs_parms *parms) {
  if (abs(ev->value-parms->min)<parms->flat)
    return 0;
  else if (ev->value>parms->max)
    return UCHAR_MAX;
  else
    return (ev->value - parms->flat - parms->min) * UCHAR_MAX / (parms->diff - parms->flat);
}

static int evdev_convert_value_direction(struct input_event *ev, struct input_device *dev, struct input_abs_parms *parms, bool reverse) {
  if (ev->value > (parms->avg+parms->range/4))
    return reverse?-1:1;
  else if (ev->value < (parms->avg-parms->range/4))
    return reverse?1:-1;
  else
    return 0;
}

static bool evdev_handle_event(struct input_event *ev, struct input_device *dev) {
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
      LiSendMultiControllerEvent(dev->controllerId, assignedControllerIds, dev->buttonFlags, dev->leftTrigger, dev->rightTrigger, dev->leftStickX, dev->leftStickY, dev->rightStickX, dev->rightStickY);
      dev->gamepadModified = false;
    }
    break;
  case EV_KEY:
    if (ev->code < sizeof(keyCodes)/sizeof(keyCodes[0])) {
      char modifier = 0;
      switch (ev->code) {
      case KEY_LEFTSHIFT:
      case KEY_RIGHTSHIFT:
        modifier = MODIFIER_SHIFT;
        break;
      case KEY_LEFTALT:
      case KEY_RIGHTALT:
        modifier = MODIFIER_ALT;
        break;
      case KEY_LEFTCTRL:
      case KEY_RIGHTCTRL:
        modifier = MODIFIER_CTRL;
        break;
      }
      if (modifier != 0) {
        if (ev->value)
          dev->modifiers |= modifier;
        else
          dev->modifiers &= ~modifier;
      }

      // Quit the stream if all the required quit keys are down
      if ((dev->modifiers & QUIT_MODIFIERS) == QUIT_MODIFIERS &&
          ev->code == QUIT_KEY && ev->value != 0) {
        return false;
      }

      short code = 0x80 << 8 | keyCodes[ev->code];
      LiSendKeyboardEvent(code, ev->value?KEY_ACTION_DOWN:KEY_ACTION_UP, dev->modifiers);
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

      if (mouseCode != 0) {
        LiSendMouseButtonEvent(ev->value?BUTTON_ACTION_PRESS:BUTTON_ACTION_RELEASE, mouseCode);
      } else {
        gamepadModified = true;

        if (gamepadCode != 0) {
          if (ev->value)
            dev->buttonFlags |= gamepadCode;
          else
            dev->buttonFlags &= ~gamepadCode;
        } else if (ev->code == dev->map.btn_tl2)
          dev->leftTrigger = ev->value?UCHAR_MAX:0;
        else if (ev->code == dev->map.btn_tr2)
          dev->rightTrigger = ev->value?UCHAR_MAX:0;
        else {
          fprintf(stderr, "Unmapped button: %d\n", ev->code);
          gamepadModified = false;
        }
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
      case REL_WHEEL:
        dev->mouseScroll = ev->value;
        break;
    }
    break;
  case EV_ABS:
    gamepadModified = true;
    if (ev->code == dev->map.abs_x)
      dev->leftStickX = evdev_convert_value(ev, dev, &dev->xParms, dev->map.reverse_x);
    else if (ev->code == dev->map.abs_y)
      dev->leftStickY = evdev_convert_value(ev, dev, &dev->yParms, dev->map.reverse_y);
    else if (ev->code == dev->map.abs_rx)
      dev->rightStickX = evdev_convert_value(ev, dev, &dev->rxParms, dev->map.reverse_rx);
    else if (ev->code == dev->map.abs_ry)
      dev->rightStickY = evdev_convert_value(ev, dev, &dev->ryParms, dev->map.reverse_ry);
    else if (ev->code == dev->map.abs_z)
      dev->leftTrigger = evdev_convert_value_byte(ev, dev, &dev->zParms);
    else if (ev->code == dev->map.abs_rz)
      dev->rightTrigger = evdev_convert_value_byte(ev, dev, &dev->rzParms);
    else if (ev->code == dev->map.abs_dpad_x) {
      int dir = evdev_convert_value_direction(ev, dev, &dev->dpadxParms, dev->map.reverse_dpad_x);
      if (dir == 1) {
        dev->buttonFlags |= RIGHT_FLAG;
        dev->buttonFlags &= ~LEFT_FLAG;
      } else if (dir == 0) {
        dev->buttonFlags &= ~RIGHT_FLAG;
        dev->buttonFlags &= ~LEFT_FLAG;
      } else {
        dev->buttonFlags &= ~RIGHT_FLAG;
        dev->buttonFlags |= LEFT_FLAG;
      }
    } else if (ev->code == dev->map.abs_dpad_y) {
      int dir = evdev_convert_value_direction(ev, dev, &dev->dpadyParms, dev->map.reverse_dpad_y);
      if (dir == 1) {
        dev->buttonFlags |= DOWN_FLAG;
        dev->buttonFlags &= ~UP_FLAG;
      } else if (dir == 0) {
        dev->buttonFlags &= ~DOWN_FLAG;
        dev->buttonFlags &= ~UP_FLAG;
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

static bool evdev_handle_mapping_event(struct input_event *ev, struct input_device *dev) {
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
      evdev_init_parms(dev, &parms, ev->code);

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

static void evdev_drain(void) {
  for (int i = 0; i < numDevices; i++) {
    struct input_event ev;
    while (libevdev_next_event(devices[i].dev, LIBEVDEV_READ_FLAG_NORMAL, &ev) >= 0);
  }
}

static int evdev_handle(int fd) {
  for (int i=0;i<numDevices;i++) {
    if (devices[i].fd = fd) {
      int rc;
      struct input_event ev;
      while ((rc = libevdev_next_event(devices[i].dev, LIBEVDEV_READ_FLAG_NORMAL, &ev)) >= 0) {
        if (rc == LIBEVDEV_READ_STATUS_SYNC)
          fprintf(stderr, "Error: cannot keep up\n");
        else if (rc == LIBEVDEV_READ_STATUS_SUCCESS) {
          if (!handler(&ev, &devices[i]))
            return LOOP_RETURN;
        }
      }
      if (rc == -ENODEV) {
        evdev_remove(i);
      } else if (rc != -EAGAIN && rc < 0) {
        fprintf(stderr, "Error: %s\n", strerror(-rc));
        exit(EXIT_FAILURE);
      }
    }
  }
  return LOOP_OK;
}

void evdev_create(const char* device, char* mapFile) {
  int fd = open(device, O_RDONLY|O_NONBLOCK);
  if (fd <= 0) {
    fprintf(stderr, "Failed to open device %s\n", device);
    fflush(stderr);
    return;
  }

  int dev = numDevices;
  numDevices++;

  if (devices == NULL) {
    devices = malloc(sizeof(struct input_device));
  } else {
    devices = realloc(devices, sizeof(struct input_device)*numDevices);
  }

  if (devices == NULL) {
    fprintf(stderr, "Not enough memory\n");
    exit(EXIT_FAILURE);
  }

  memset(&devices[dev], 0, sizeof(devices[0]));
  devices[dev].fd = fd;
  devices[dev].dev = libevdev_new();
  libevdev_set_fd(devices[dev].dev, devices[dev].fd);

  if (mapFile != NULL)
    mapping_load(mapFile, &(devices[dev].map));

  devices[dev].controllerId = -1;
  evdev_init_parms(&devices[dev], &(devices[dev].xParms), devices[dev].map.abs_x);
  evdev_init_parms(&devices[dev], &(devices[dev].yParms), devices[dev].map.abs_y);
  evdev_init_parms(&devices[dev], &(devices[dev].zParms), devices[dev].map.abs_z);
  evdev_init_parms(&devices[dev], &(devices[dev].rxParms), devices[dev].map.abs_rx);
  evdev_init_parms(&devices[dev], &(devices[dev].ryParms), devices[dev].map.abs_ry);
  evdev_init_parms(&devices[dev], &(devices[dev].rzParms), devices[dev].map.abs_rz);
  evdev_init_parms(&devices[dev], &(devices[dev].dpadxParms), devices[dev].map.abs_dpad_x);
  evdev_init_parms(&devices[dev], &(devices[dev].dpadyParms), devices[dev].map.abs_dpad_y);

  if (grabbingDevices) {
    if (ioctl(fd, EVIOCGRAB, 1) < 0) {
      fprintf(stderr, "EVIOCGRAB failed with error %d\n", errno);
    }
  }

  loop_add_fd(devices[dev].fd, &evdev_handle, POLLIN);
}

static void evdev_map_key(char* keyName, short* key) {
  printf("Press %s\n", keyName);
  fflush(stdout)
  currentKey = key;
  currentAbs = NULL;
  *key = -1;
  loop_main();

  usleep(250000);
  evdev_drain();
}

static void evdev_map_abs(char* keyName, short* abs, bool* reverse) {
  printf("Move %s\n", keyName);
  fflush(stdout)
  currentKey = NULL;
  currentAbs = abs;
  currentReverse = reverse;
  *abs = -1;
  loop_main();

  usleep(250000);
  evdev_drain();
}

static void evdev_map_abskey(char* keyName, short* key, short* abs, bool* reverse) {
  printf("Press %s\n", keyName);
  fflush(stdout)
  currentKey = key;
  currentAbs = abs;
  currentReverse = reverse;
  *key = -1;
  *abs = -1;
  *currentReverse = false;
  loop_main();

  usleep(250000);
  evdev_drain();
}

void evdev_map(char* fileName) {
  struct mapping map;

  handler = evdev_handle_mapping_event;

  evdev_map_abs("Left Stick Right", &(map.abs_x), &(map.reverse_x));
  evdev_map_abs("Left Stick Up", &(map.abs_y), &(map.reverse_y));
  evdev_map_key("Left Stick Button", &(map.btn_thumbl));

  evdev_map_abs("Right Stick Right", &(map.abs_rx), &(map.reverse_rx));
  evdev_map_abs("Right Stick Up", &(map.abs_ry), &(map.reverse_ry));
  evdev_map_key("Right Stick Button", &(map.btn_thumbr));

  evdev_map_abskey("D-Pad Right", &(map.btn_dpad_right), &(map.abs_dpad_x), &(map.reverse_dpad_x));
  if (map.btn_dpad_right >= 0)
    evdev_map_key("D-Pad Left", &(map.btn_dpad_left));
  else
    map.btn_dpad_left = -1;

  evdev_map_abskey("D-Pad Down", &(map.btn_dpad_down), &(map.abs_dpad_y), &(map.reverse_dpad_y));
  if (map.btn_dpad_down >= 0)
    evdev_map_key("D-Pad Up", &(map.btn_dpad_up));
  else
    map.btn_dpad_up = -1;

  evdev_map_key("Button X (1)", &(map.btn_west));
  evdev_map_key("Button A (2)", &(map.btn_south));
  evdev_map_key("Button B (3)", &(map.btn_east));
  evdev_map_key("Button Y (4)", &(map.btn_north));
  evdev_map_key("Back Button", &(map.btn_select));
  evdev_map_key("Start Button", &(map.btn_start));
  evdev_map_key("Special Button", &(map.btn_mode));

  bool ignored;
  evdev_map_abskey("Left Trigger", &(map.btn_tl2), &(map.abs_z), &ignored);
  evdev_map_abskey("Right Trigger", &(map.btn_tr2), &(map.abs_rz), &ignored);

  evdev_map_key("Left Bumper", &(map.btn_tl));
  evdev_map_key("Right Bumper", &(map.btn_tr));
  mapping_save(fileName, &map);
}

void evdev_start() {
  // After grabbing, the only way to quit via the keyboard
  // is via the special key combo that the input handling
  // code looks for. For this reason, we wait to grab until
  // we're ready to take input events. Ctrl+C works up until
  // this point.
  for (int i = 0; i < numDevices; i++) {
    if (ioctl(devices[i].fd, EVIOCGRAB, 1) < 0) {
      fprintf(stderr, "EVIOCGRAB failed with error %d\n", errno);
    }
  }

  // Any new input devices detected after this point will be grabbed immediately
  grabbingDevices = true;

  // Handle input events until the quit combo is pressed
}

void evdev_stop() {
  evdev_drain();
}

void evdev_init() {
  handler = evdev_handle_event;
}
