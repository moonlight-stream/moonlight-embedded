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

#include "evdev.h"

#include "keyboard.h"

#include "../loop.h"

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
#include <endian.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define int16_to_le(val) val
#else
#define int16_to_le(val) ((((val) >> 8) & 0x00FF) | (((val) << 8) & 0xFF00))
#endif

struct input_abs_parms {
  int min, max;
  int flat;
  int avg;
  int range, diff;
};

struct input_device {
  struct libevdev *dev;
  struct mapping* map;
  int key_map[KEY_MAX];
  int abs_map[ABS_MAX];
  int hats_state[3][2];
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
};

#define HAT_UP 1
#define HAT_RIGHT 2
#define HAT_DOWN 4
#define HAT_LEFT 8
static const int hat_constants[3][3] = {{HAT_UP | HAT_LEFT, HAT_UP, HAT_UP | HAT_RIGHT}, {HAT_LEFT, 0, HAT_RIGHT}, {HAT_LEFT | HAT_DOWN, HAT_DOWN, HAT_DOWN | HAT_RIGHT}};

#define set_hat(flags, flag, hat, hat_flag) flags = (hat & hat_flag) == hat_flag ? flags | flag : flags & ~flag

static struct input_device* devices = NULL;
static int numDevices = 0;
static int assignedControllerIds = 0;

static short* currentKey;
static short* currentAbs;
static bool* currentReverse;

static bool grabbingDevices;

#define QUIT_MODIFIERS (MODIFIER_SHIFT|MODIFIER_ALT|MODIFIER_CTRL)
#define QUIT_KEY KEY_Q
#define QUIT_BUTTONS (PLAY_FLAG|BACK_FLAG|LB_FLAG|RB_FLAG)

static bool (*handler) (struct input_event*, struct input_device*);

static int evdev_get_map_key(int* map, int length, int value) {
  for (int i = 0; i < length; i++) {
    if (value == map[i])
      return i;
  }
  return -1;
}

static bool evdev_init_parms(struct input_device *dev, struct input_abs_parms *parms, int code) {
  int abs = evdev_get_map_key(dev->abs_map, ABS_MAX, code);

  if (abs >= 0) {
    parms->flat = libevdev_get_abs_flat(dev->dev, abs);
    parms->min = libevdev_get_abs_minimum(dev->dev, abs);
    parms->max = libevdev_get_abs_maximum(dev->dev, abs);
    if (parms->flat == 0 && parms->min == 0 && parms->max == 0)
      return false;

    parms->avg = (parms->min+parms->max)/2;
    parms->range = parms->max - parms->avg;
    parms->diff = parms->max - parms->min;
  }
  return true;
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
  if (parms->max == 0 && parms->min == 0) {
    fprintf(stderr, "Axis not found: %d\n", ev->code);
    return 0;
  }

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
  if (parms->max == 0 && parms->min == 0) {
    fprintf(stderr, "Axis not found: %d\n", ev->code);
    return 0;
  }

  if (abs(ev->value-parms->min)<parms->flat)
    return 0;
  else if (ev->value>parms->max)
    return UCHAR_MAX;
  else
    return (ev->value - parms->flat - parms->min) * UCHAR_MAX / (parms->diff - parms->flat);
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
      int index = ev->code > BTN_MISC && ev->code < (BTN_MISC + KEY_MAX) ? dev->key_map[ev->code - BTN_MISC] : -1;

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
        if (dev->map == NULL)
          break;
        else if (index == dev->map->btn_a)
          gamepadCode = A_FLAG;
        else if (index == dev->map->btn_x)
          gamepadCode = X_FLAG;
        else if (index == dev->map->btn_y)
          gamepadCode = Y_FLAG;
        else if (index == dev->map->btn_b)
          gamepadCode = B_FLAG;
        else if (index == dev->map->btn_dpup)
          gamepadCode = UP_FLAG;
        else if (index == dev->map->btn_dpdown)
          gamepadCode = DOWN_FLAG;
        else if (index == dev->map->btn_dpright)
          gamepadCode = RIGHT_FLAG;
        else if (index == dev->map->btn_dpleft)
          gamepadCode = LEFT_FLAG;
        else if (index == dev->map->btn_leftstick)
          gamepadCode = LS_CLK_FLAG;
        else if (index == dev->map->btn_rightstick)
          gamepadCode = RS_CLK_FLAG;
        else if (index == dev->map->btn_leftshoulder)
          gamepadCode = LB_FLAG;
        else if (index == dev->map->btn_rightshoulder)
          gamepadCode = RB_FLAG;
        else if (index == dev->map->btn_start)
          gamepadCode = PLAY_FLAG;
        else if (index == dev->map->btn_back)
          gamepadCode = BACK_FLAG;
        else if (index == dev->map->btn_guide)
          gamepadCode = SPECIAL_FLAG;
      }

      if (mouseCode != 0) {
        LiSendMouseButtonEvent(ev->value?BUTTON_ACTION_PRESS:BUTTON_ACTION_RELEASE, mouseCode);
      } else if (gamepadCode != 0) {
        gamepadModified = true;

        if (ev->value)
          dev->buttonFlags |= gamepadCode;
        else
          dev->buttonFlags &= ~gamepadCode;
      } else if (dev->map != NULL && index == dev->map->btn_lefttrigger) {
        dev->leftTrigger = ev->value ? UCHAR_MAX : 0;
        gamepadModified = true;
      } else if (dev->map != NULL && index == dev->map->btn_righttrigger) {
        dev->rightTrigger = ev->value ? UCHAR_MAX : 0;
        gamepadModified = true;
      } else {
        if (dev->map != NULL)
          fprintf(stderr, "Unmapped button: %d\n", ev->code);

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
      case REL_WHEEL:
        dev->mouseScroll = ev->value;
        break;
    }
    break;
  case EV_ABS:
    if (dev->map == NULL)
      break;

    gamepadModified = true;
    int index = dev->abs_map[ev->code];
    int hat_index = (ev->code - ABS_HAT0X) / 2;
    int hat_dir_index = (ev->code - ABS_HAT0X) % 2;

    switch (ev->code) {
    case ABS_HAT0X:
    case ABS_HAT0Y:
    case ABS_HAT1X:
    case ABS_HAT1Y:
    case ABS_HAT2X:
    case ABS_HAT2Y:
    case ABS_HAT3X:
    case ABS_HAT3Y:
      dev->hats_state[hat_index][hat_dir_index] = ev->value < 0 ? -1 : (ev->value == 0 ? 0 : 1);
      int hat_state = hat_constants[dev->hats_state[hat_index][1] + 1][dev->hats_state[hat_index][0] + 1];
      if (hat_index == dev->map->hat_dpup)
        set_hat(dev->buttonFlags, UP_FLAG, hat_state, dev->map->hat_dir_dpup);
      if (hat_index == dev->map->hat_dpdown)
        set_hat(dev->buttonFlags, DOWN_FLAG, hat_state, dev->map->hat_dir_dpdown);
      if (hat_index == dev->map->hat_dpright)
        set_hat(dev->buttonFlags, RIGHT_FLAG, hat_state, dev->map->hat_dir_dpright);
      if (hat_index == dev->map->hat_dpleft)
        set_hat(dev->buttonFlags, LEFT_FLAG, hat_state, dev->map->hat_dir_dpleft);
      break;
    default:
      if (index == dev->map->abs_leftx)
        dev->leftStickX = evdev_convert_value(ev, dev, &dev->xParms, dev->map->reverse_leftx);
      else if (index == dev->map->abs_lefty)
        dev->leftStickY = evdev_convert_value(ev, dev, &dev->yParms, !dev->map->reverse_lefty);
      else if (index == dev->map->abs_rightx)
        dev->rightStickX = evdev_convert_value(ev, dev, &dev->rxParms, dev->map->reverse_rightx);
      else if (index == dev->map->abs_righty)
        dev->rightStickY = evdev_convert_value(ev, dev, &dev->ryParms, !dev->map->reverse_righty);
      else if (index == dev->map->abs_lefttrigger)
        dev->leftTrigger = evdev_convert_value_byte(ev, dev, &dev->zParms);
      else if (index == dev->map->abs_righttrigger)
        dev->rightTrigger = evdev_convert_value_byte(ev, dev, &dev->rzParms);
      else
        gamepadModified = false;
    }
  }

  if (gamepadModified && (dev->buttonFlags & QUIT_BUTTONS) == QUIT_BUTTONS)
    return false;

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

void evdev_create(const char* device, struct mapping* mappings, bool verbose) {
  int fd = open(device, O_RDONLY|O_NONBLOCK);
  if (fd <= 0) {
    fprintf(stderr, "Failed to open device %s\n", device);
    fflush(stderr);
    return;
  }

  struct libevdev *evdev = libevdev_new();
  libevdev_set_fd(evdev, fd);
  const char* name = libevdev_get_name(evdev);

  int16_t guid[8] = {0};
  guid[0] = int16_to_le(libevdev_get_id_bustype(evdev));
  guid[2] = int16_to_le(libevdev_get_id_vendor(evdev));
  guid[4] = int16_to_le(libevdev_get_id_product(evdev));
  guid[6] = int16_to_le(libevdev_get_id_version(evdev));

  char str_guid[33];
  char* buf = str_guid;
  for (int i = 0; i < 16; i++)
    buf += sprintf(buf, "%02x", ((unsigned char*) guid)[i]);

  struct mapping* default_mapping = NULL;
  struct mapping* xwc_mapping = NULL;
  while (mappings != NULL) {
    if (strncmp(str_guid, mappings->guid, 32) == 0) {
      if (verbose)
        printf("Detected %s (%s) on %s as %s\n", name, str_guid, device, mappings->name);

      break;
    } else if (strncmp("default", mappings->guid, 32) == 0)
      default_mapping = mappings;
    else if (strncmp("xwc", mappings->guid, 32) == 0)
      xwc_mapping = mappings;

    mappings = mappings->next;
  }

  if (mappings == NULL && strstr(name, "Xbox 360 Wireless Receiver") != NULL)
    mappings = xwc_mapping;

  bool is_keyboard = libevdev_has_event_code(evdev, EV_KEY, KEY_Q);
  bool is_mouse = libevdev_has_event_type(evdev, EV_REL) || libevdev_has_event_code(evdev, EV_KEY, BTN_LEFT);

  if (mappings == NULL && !(is_keyboard || is_mouse)) {
    fprintf(stderr, "No mapping available for %s (%s) on %s\n", name, str_guid, device);
    mappings = default_mapping;
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
  devices[dev].dev = evdev;
  devices[dev].map = mappings;
  memset(&devices[dev].key_map, -1, sizeof(devices[dev].key_map));
  memset(&devices[dev].abs_map, -1, sizeof(devices[dev].abs_map));

  int nbuttons = 0;
  for (int i = BTN_JOYSTICK; i < KEY_MAX; ++i) {
    if (libevdev_has_event_code(devices[dev].dev, EV_KEY, i))
      devices[dev].key_map[i - BTN_MISC] = nbuttons++;
  }
  for (int i = BTN_MISC; i < BTN_JOYSTICK; ++i) {
    if (libevdev_has_event_code(devices[dev].dev, EV_KEY, i))
      devices[dev].key_map[i - BTN_MISC] = nbuttons++;
  }

  int naxes = 0;
  for (int i = 0; i < ABS_MAX; ++i) {
    /* Skip hats */
    if (i == ABS_HAT0X)
      i = ABS_HAT3Y;
    else if (libevdev_has_event_code(devices[dev].dev, EV_ABS, i))
      devices[dev].abs_map[i] = naxes++;
  }

  devices[dev].controllerId = -1;

  if (devices[dev].map != NULL) {
    bool valid = evdev_init_parms(&devices[dev], &(devices[dev].xParms), devices[dev].map->abs_leftx);
    valid &= evdev_init_parms(&devices[dev], &(devices[dev].yParms), devices[dev].map->abs_lefty);
    valid &= evdev_init_parms(&devices[dev], &(devices[dev].zParms), devices[dev].map->abs_lefttrigger);
    valid &= evdev_init_parms(&devices[dev], &(devices[dev].rxParms), devices[dev].map->abs_rightx);
    valid &= evdev_init_parms(&devices[dev], &(devices[dev].ryParms), devices[dev].map->abs_righty);
    valid &= evdev_init_parms(&devices[dev], &(devices[dev].rzParms), devices[dev].map->abs_righttrigger);
    if (!valid)
      fprintf(stderr, "Mapping for %s (%s) on %s is incorrect\n", name, str_guid, device);
  }

  if (grabbingDevices) {
    if (ioctl(fd, EVIOCGRAB, 1) < 0) {
      fprintf(stderr, "EVIOCGRAB failed with error %d\n", errno);
    }
  }

  loop_add_fd(devices[dev].fd, &evdev_handle, POLLIN);
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
