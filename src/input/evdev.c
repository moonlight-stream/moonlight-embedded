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
#include <math.h>

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
  bool is_keyboard;
  bool is_mouse;
  bool is_touchscreen;
  int rotate;
  struct mapping* map;
  int key_map[KEY_CNT];
  int abs_map[ABS_CNT];
  int hats_state[3][2];
  int fd;
  char modifiers;
  __s32 mouseDeltaX, mouseDeltaY, mouseScroll;
  __s32 touchDownX, touchDownY, touchX, touchY;
  struct timeval touchDownTime;
  struct timeval btnDownTime;
  short controllerId;
  int haptic_effect_id;
  int buttonFlags;
  char leftTrigger, rightTrigger;
  short leftStickX, leftStickY;
  short rightStickX, rightStickY;
  bool gamepadModified;
  bool mouseEmulation;
  pthread_t meThread;
  struct input_abs_parms xParms, yParms, rxParms, ryParms, zParms, rzParms;
  struct input_abs_parms leftParms, rightParms, upParms, downParms;
};

#define HAT_UP 1
#define HAT_RIGHT 2
#define HAT_DOWN 4
#define HAT_LEFT 8
static const int hat_constants[3][3] = {{HAT_UP | HAT_LEFT, HAT_UP, HAT_UP | HAT_RIGHT}, {HAT_LEFT, 0, HAT_RIGHT}, {HAT_LEFT | HAT_DOWN, HAT_DOWN, HAT_DOWN | HAT_RIGHT}};

#define set_hat(flags, flag, hat, hat_flag) flags = (hat & hat_flag) == hat_flag ? flags | flag : flags & ~flag

#define TOUCH_UP -1
#define TOUCH_CLICK_RADIUS 10
#define TOUCH_CLICK_DELAY 100000 // microseconds
#define TOUCH_RCLICK_TIME 750 // milliseconds

// How long the Start button must be pressed to toggle mouse emulation
#define MOUSE_EMULATION_LONG_PRESS_TIME 750
// How long between polling the gamepad to send virtual mouse input
#define MOUSE_EMULATION_POLLING_INTERVAL 50000
// Determines how fast the mouse will move each interval
#define MOUSE_EMULATION_MOTION_MULTIPLIER 3
// Determines the maximum motion amount before allowing movement
#define MOUSE_EMULATION_DEADZONE 2

static struct input_device* devices = NULL;
static int numDevices = 0;
static int assignedControllerIds = 0;

static short* currentKey;
static short* currentHat;
static short* currentHatDir;
static short* currentAbs;
static bool* currentReverse;

static bool grabbingDevices;
static bool mouseEmulationEnabled;

static bool waitingToExitOnModifiersUp = false;

int evdev_gamepads = 0;

#define ACTION_MODIFIERS (MODIFIER_SHIFT|MODIFIER_ALT|MODIFIER_CTRL)
#define QUIT_KEY KEY_Q
#define QUIT_BUTTONS (PLAY_FLAG|BACK_FLAG|LB_FLAG|RB_FLAG)

static bool (*handler) (struct input_event*, struct input_device*);

static int evdev_get_map(int* map, int length, int value) {
  for (int i = 0; i < length; i++) {
    if (value == map[i])
      return i;
  }
  return -1;
}

static bool evdev_init_parms(struct input_device *dev, struct input_abs_parms *parms, int code) {
  int abs = evdev_get_map(dev->abs_map, ABS_MAX, code);

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

  printf("Input device removed: %s (player %d)\n", libevdev_get_name(devices[devindex].dev), devices[devindex].controllerId + 1);

  if (devices[devindex].controllerId >= 0) {
    assignedControllerIds &= ~(1 << devices[devindex].controllerId);
    LiSendMultiControllerEvent(devices[devindex].controllerId, assignedControllerIds, 0, 0, 0, 0, 0, 0, 0);
  }
  if (devices[devindex].mouseEmulation) {
    devices[devindex].mouseEmulation = false;
    pthread_join(devices[devindex].meThread, NULL);
  }

  if (devindex != numDevices && numDevices > 0)
    memcpy(&devices[devindex], &devices[numDevices], sizeof(struct input_device));
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

static char evdev_convert_value_byte(struct input_event *ev, struct input_device *dev, struct input_abs_parms *parms, char halfaxis) {
  if (parms->max == 0 && parms->min == 0) {
    fprintf(stderr, "Axis not found: %d\n", ev->code);
    return 0;
  }

  if (halfaxis == 0) {
    if (abs(ev->value-parms->min)<parms->flat)
      return 0;
    else if (ev->value>parms->max)
      return UCHAR_MAX;
    else
      return (ev->value - parms->flat - parms->min) * UCHAR_MAX / (parms->diff - parms->flat);
  } else {
    short val = evdev_convert_value(ev, dev, parms, false);
    if (halfaxis == '-' && val < 0)
      return -(int)val * UCHAR_MAX / (SHRT_MAX-SHRT_MIN);
    else if (halfaxis == '+' && val > 0)
      return (int)val * UCHAR_MAX / (SHRT_MAX-SHRT_MIN);
    else
      return 0;
  }
}

void *HandleMouseEmulation(void* param)
{
  struct input_device* dev = (struct input_device*) param;

  while (dev->mouseEmulation) {
    usleep(MOUSE_EMULATION_POLLING_INTERVAL);

    short rawX;
    short rawY;

    // Determine which analog stick is currently receiving the strongest input
    if ((uint32_t)abs(dev->leftStickX) + abs(dev->leftStickY) > (uint32_t)abs(dev->rightStickX) + abs(dev->rightStickY)) {
      rawX = dev->leftStickX;
      rawY = dev->leftStickY;
    } else {
      rawX = dev->rightStickX;
      rawY = dev->rightStickY;
    }

    float deltaX;
    float deltaY;

    // Produce a base vector for mouse movement with increased speed as we deviate further from center
    deltaX = pow((float)rawX / 32767.0f * MOUSE_EMULATION_MOTION_MULTIPLIER, 3);
    deltaY = pow((float)rawY / 32767.0f * MOUSE_EMULATION_MOTION_MULTIPLIER, 3);

    // Enforce deadzones
    deltaX = abs(deltaX) > MOUSE_EMULATION_DEADZONE ? deltaX - MOUSE_EMULATION_DEADZONE : 0;
    deltaY = abs(deltaY) > MOUSE_EMULATION_DEADZONE ? deltaY - MOUSE_EMULATION_DEADZONE : 0;

    if (deltaX != 0 || deltaY != 0)
      LiSendMouseMoveEvent(deltaX, -deltaY);
  }
}

static bool evdev_handle_event(struct input_event *ev, struct input_device *dev) {
  bool gamepadModified = false;

  switch (ev->type) {
  case EV_SYN:
    if (dev->mouseDeltaX != 0 || dev->mouseDeltaY != 0) {
      switch (dev->rotate) {
      case 90:
        LiSendMouseMoveEvent(dev->mouseDeltaY, -dev->mouseDeltaX);
        break;
      case 180:
        LiSendMouseMoveEvent(-dev->mouseDeltaX, -dev->mouseDeltaY);
        break;
      case 270:
        LiSendMouseMoveEvent(-dev->mouseDeltaY, dev->mouseDeltaX);
        break;
      default:
        LiSendMouseMoveEvent(dev->mouseDeltaX, dev->mouseDeltaY);
        break;
      }
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
            printf("Assigned %s as player %d\n", libevdev_get_name(dev->dev), i+1);
            break;
          }
        }
        //Use id 0 when too many gamepads are connected
        if (dev->controllerId < 0)
          dev->controllerId = 0;
      }
      // Send event only if mouse emulation is disabled.
      if (dev->mouseEmulation == false)
        LiSendMultiControllerEvent(dev->controllerId, assignedControllerIds, dev->buttonFlags, dev->leftTrigger, dev->rightTrigger, dev->leftStickX, dev->leftStickY, dev->rightStickX, dev->rightStickY);
      dev->gamepadModified = false;
    }
    break;
  case EV_KEY:
    if (ev->code > KEY_MAX)
      return true;
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
      case KEY_LEFTMETA:
      case KEY_RIGHTMETA:
        modifier = MODIFIER_META;
        break;
      }
      if (modifier != 0) {
        if (ev->value)
          dev->modifiers |= modifier;
        else
          dev->modifiers &= ~modifier;
      }

      // After the quit key combo is pressed, quit once all keys are raised
      if ((dev->modifiers & ACTION_MODIFIERS) == ACTION_MODIFIERS &&
          ev->code == QUIT_KEY && ev->value != 0) {
        waitingToExitOnModifiersUp = true;
        return true;
      } else if (waitingToExitOnModifiersUp && dev->modifiers == 0)
        return false;

      short code = 0x80 << 8 | keyCodes[ev->code];
      LiSendKeyboardEvent(code, ev->value?KEY_ACTION_DOWN:KEY_ACTION_UP, dev->modifiers);
    } else {
      int mouseCode = 0;
      short gamepadCode = 0;
      int index = dev->key_map[ev->code];

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
      case BTN_SIDE:
        mouseCode = BUTTON_X1;
        break;
      case BTN_EXTRA:
        mouseCode = BUTTON_X2;
        break;
      case BTN_TOUCH:
        if (ev->value == 1) {
          dev->touchDownTime = ev->time;
        } else {
          if (dev->touchDownX != TOUCH_UP && dev->touchDownY != TOUCH_UP) {
            int deltaX = dev->touchX - dev->touchDownX;
            int deltaY = dev->touchY - dev->touchDownY;
            if (deltaX * deltaX + deltaY * deltaY < TOUCH_CLICK_RADIUS * TOUCH_CLICK_RADIUS) {
              struct timeval elapsedTime;
              timersub(&ev->time, &dev->touchDownTime, &elapsedTime);
              int holdTimeMs = elapsedTime.tv_sec * 1000 + elapsedTime.tv_usec / 1000;
              int button = holdTimeMs >= TOUCH_RCLICK_TIME ? BUTTON_RIGHT : BUTTON_LEFT;
              LiSendMouseButtonEvent(BUTTON_ACTION_PRESS, button);
              usleep(TOUCH_CLICK_DELAY);
              LiSendMouseButtonEvent(BUTTON_ACTION_RELEASE, button);
            }
          }
          dev->touchDownX = TOUCH_UP;
          dev->touchDownY = TOUCH_UP;
        }
        break;
      default:
        gamepadModified = true;
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
        gamepadModified = false;
      } else if (gamepadCode != 0) {
        if (ev->value) {
          dev->buttonFlags |= gamepadCode;
          dev->btnDownTime = ev->time;
        } else
          dev->buttonFlags &= ~gamepadCode;

        if (mouseEmulationEnabled && gamepadCode == PLAY_FLAG && ev->value == 0) {
          struct timeval elapsedTime;
          timersub(&ev->time, &dev->btnDownTime, &elapsedTime);
          int holdTimeMs = elapsedTime.tv_sec * 1000 + elapsedTime.tv_usec / 1000;
          if (holdTimeMs >= MOUSE_EMULATION_LONG_PRESS_TIME) {
            if (dev->mouseEmulation) {
              dev->mouseEmulation = false;
              pthread_join(dev->meThread, NULL);
              dev->meThread = 0;
              printf("Mouse emulation disabled for controller %d.\n", dev->controllerId);
            } else {
              dev->mouseEmulation = true;
              pthread_create(&dev->meThread, NULL, HandleMouseEmulation, dev);
              printf("Mouse emulation enabled for controller %d.\n", dev->controllerId);
            }
            // clear gamepad state.
            LiSendMultiControllerEvent(dev->controllerId, assignedControllerIds, 0, 0, 0, 0, 0, 0, 0);
          }
        } else if (dev->mouseEmulation) {
          char action = ev->value ? BUTTON_ACTION_PRESS : BUTTON_ACTION_RELEASE;
          switch (gamepadCode) {
            case A_FLAG:
              LiSendMouseButtonEvent(action, BUTTON_LEFT);
              break;
            case B_FLAG:
              LiSendMouseButtonEvent(action, BUTTON_RIGHT);
              break;
            case X_FLAG:
              LiSendMouseButtonEvent(action, BUTTON_MIDDLE);
              break;
            case LB_FLAG:
              LiSendMouseButtonEvent(action, BUTTON_X1);
              break;
            case RB_FLAG:
              LiSendMouseButtonEvent(action, BUTTON_X2);
              break;
          }
        }
      } else if (dev->map != NULL && index == dev->map->btn_lefttrigger)
        dev->leftTrigger = ev->value ? UCHAR_MAX : 0;
      else if (dev->map != NULL && index == dev->map->btn_righttrigger)
        dev->rightTrigger = ev->value ? UCHAR_MAX : 0;
      else {
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
    if (ev->code > ABS_MAX)
      return true;
    if (dev->is_touchscreen) {
      switch (ev->code) {
      case ABS_X:
        if (dev->touchDownX == TOUCH_UP) {
          dev->touchDownX = ev->value;
          dev->touchX = ev->value;
        } else {
          dev->mouseDeltaX += (ev->value - dev->touchX);
          dev->touchX = ev->value;
        }
        break;
      case ABS_Y:
        if (dev->touchDownY == TOUCH_UP) {
          dev->touchDownY = ev->value;
          dev->touchY = ev->value;
        } else {
          dev->mouseDeltaY += (ev->value - dev->touchY);
          dev->touchY = ev->value;
        }
        break;
      }
      break;
    }

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
      else
        gamepadModified = false;

      if (index == dev->map->abs_lefttrigger) {
        dev->leftTrigger = evdev_convert_value_byte(ev, dev, &dev->zParms, dev->map->halfaxis_lefttrigger);
        gamepadModified = true;
      }
      if (index == dev->map->abs_righttrigger) {
        dev->rightTrigger = evdev_convert_value_byte(ev, dev, &dev->rzParms, dev->map->halfaxis_righttrigger);
        gamepadModified = true;
      }

      if (index == dev->map->abs_dpright) {
        if (evdev_convert_value_byte(ev, dev, &dev->rightParms, dev->map->halfaxis_dpright) > 127)
          dev->buttonFlags |= RIGHT_FLAG;
        else
          dev->buttonFlags &= ~RIGHT_FLAG;

        gamepadModified = true;
      }
      if (index == dev->map->abs_dpleft) {
        if (evdev_convert_value_byte(ev, dev, &dev->leftParms, dev->map->halfaxis_dpleft) > 127)
          dev->buttonFlags |= LEFT_FLAG;
        else
          dev->buttonFlags &= ~LEFT_FLAG;

        gamepadModified = true;
      }
      if (index == dev->map->abs_dpup) {
        if (evdev_convert_value_byte(ev, dev, &dev->upParms, dev->map->halfaxis_dpup) > 127)
          dev->buttonFlags |= UP_FLAG;
        else
          dev->buttonFlags &= ~UP_FLAG;

        gamepadModified = true;
      }
      if (index == dev->map->abs_dpdown) {
        if (evdev_convert_value_byte(ev, dev, &dev->downParms, dev->map->halfaxis_dpdown) > 127)
          dev->buttonFlags |= DOWN_FLAG;
        else
          dev->buttonFlags &= ~DOWN_FLAG;

        gamepadModified = true;
      }
    }
  }

  if (gamepadModified && (dev->buttonFlags & QUIT_BUTTONS) == QUIT_BUTTONS) {
    LiSendMultiControllerEvent(dev->controllerId, assignedControllerIds, 0, 0, 0, 0, 0, 0, 0);
    return false;
  }

  dev->gamepadModified |= gamepadModified;
  return true;
}

static bool evdev_handle_mapping_event(struct input_event *ev, struct input_device *dev) {
  int index, hat_index;
  switch (ev->type) {
  case EV_KEY:
    index = dev->key_map[ev->code];
    if (currentKey != NULL) {
      if (ev->value)
        *currentKey = index;
      else if (*currentKey != -1 && index == *currentKey)
        return false;
    }
    break;
  case EV_ABS:
    hat_index = (ev->code - ABS_HAT0X) / 2;
    if (hat_index >= 0 && hat_index < 4) {
      int hat_dir_index = (ev->code - ABS_HAT0X) % 2;
      dev->hats_state[hat_index][hat_dir_index] = ev->value < 0 ? -1 : (ev->value == 0 ? 0 : 1);
    }
    if (currentAbs != NULL) {
      struct input_abs_parms parms;
      evdev_init_parms(dev, &parms, ev->code);

      if (ev->value > parms.avg + parms.range/2) {
        *currentAbs = dev->abs_map[ev->code];
        *currentReverse = false;
      } else if (ev->value < parms.avg - parms.range/2) {
        *currentAbs = dev->abs_map[ev->code];
        *currentReverse = true;
      } else if (ev->code == *currentAbs)
        return false;
    } else if (currentHat != NULL) {
      if (hat_index >= 0 && hat_index < 4) {
        *currentHat = hat_index;
        *currentHatDir = hat_constants[dev->hats_state[hat_index][1] + 1][dev->hats_state[hat_index][0] + 1];
        return false;
      }
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
    if (devices[i].fd == fd) {
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

void evdev_create(const char* device, struct mapping* mappings, bool verbose, int rotate) {
  int fd = open(device, O_RDWR|O_NONBLOCK);
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
  int16_t vendor = libevdev_get_id_vendor(evdev);
  int16_t product = libevdev_get_id_product(evdev);
  if (vendor && product) {
    guid[2] = int16_to_le(vendor);
    guid[4] = int16_to_le(product);
    guid[6] = int16_to_le(libevdev_get_id_version(evdev));
  } else
    strncpy((char*) &guid[2], name, 11);

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
  bool is_touchscreen = libevdev_has_event_code(evdev, EV_KEY, BTN_TOUCH);

  // This classification logic comes from SDL
  bool is_accelerometer =
    ((libevdev_has_event_code(evdev, EV_ABS, ABS_X) &&
      libevdev_has_event_code(evdev, EV_ABS, ABS_Y) &&
      libevdev_has_event_code(evdev, EV_ABS, ABS_Z)) ||
     (libevdev_has_event_code(evdev, EV_ABS, ABS_RX) &&
      libevdev_has_event_code(evdev, EV_ABS, ABS_RY) &&
      libevdev_has_event_code(evdev, EV_ABS, ABS_RZ))) &&
    !libevdev_has_event_type(evdev, EV_KEY);
  bool is_gamepad =
    libevdev_has_event_code(evdev, EV_ABS, ABS_X) &&
    libevdev_has_event_code(evdev, EV_ABS, ABS_Y) &&
    (libevdev_has_event_code(evdev, EV_KEY, BTN_TRIGGER) ||
     libevdev_has_event_code(evdev, EV_KEY, BTN_A) ||
     libevdev_has_event_code(evdev, EV_KEY, BTN_1) ||
     libevdev_has_event_code(evdev, EV_ABS, ABS_RX) ||
     libevdev_has_event_code(evdev, EV_ABS, ABS_RY) ||
     libevdev_has_event_code(evdev, EV_ABS, ABS_RZ) ||
     libevdev_has_event_code(evdev, EV_ABS, ABS_THROTTLE) ||
     libevdev_has_event_code(evdev, EV_ABS, ABS_RUDDER) ||
     libevdev_has_event_code(evdev, EV_ABS, ABS_WHEEL) ||
     libevdev_has_event_code(evdev, EV_ABS, ABS_GAS) ||
     libevdev_has_event_code(evdev, EV_ABS, ABS_BRAKE));

  if (is_accelerometer) {
    if (verbose)
      printf("Ignoring accelerometer: %s\n", name);
    libevdev_free(evdev);
    close(fd);
    return;
  }

  if (is_gamepad) {
    evdev_gamepads++;

    if (mappings == NULL) {
      fprintf(stderr, "No mapping available for %s (%s) on %s\n", name, str_guid, device);
      mappings = default_mapping;
    }
  } else {
    if (verbose)
      printf("Not mapping %s as a gamepad\n", name);
    mappings = NULL;
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
  /* Set unused evdev indices to -2 to avoid aliasing with the default -1 in our mappings */
  memset(&devices[dev].key_map, -2, sizeof(devices[dev].key_map));
  memset(&devices[dev].abs_map, -2, sizeof(devices[dev].abs_map));
  devices[dev].is_keyboard = is_keyboard;
  devices[dev].is_mouse = is_mouse;
  devices[dev].is_touchscreen = is_touchscreen;
  devices[dev].rotate = rotate;
  devices[dev].touchDownX = TOUCH_UP;
  devices[dev].touchDownY = TOUCH_UP;

  int nbuttons = 0;
  /* Count joystick buttons first like SDL does */
  for (int i = BTN_JOYSTICK; i < KEY_MAX; ++i) {
    if (libevdev_has_event_code(devices[dev].dev, EV_KEY, i))
      devices[dev].key_map[i] = nbuttons++;
  }
  for (int i = 0; i < BTN_JOYSTICK; ++i) {
    if (libevdev_has_event_code(devices[dev].dev, EV_KEY, i))
      devices[dev].key_map[i] = nbuttons++;
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
  devices[dev].haptic_effect_id = -1;

  if (devices[dev].map != NULL) {
    bool valid = evdev_init_parms(&devices[dev], &(devices[dev].xParms), devices[dev].map->abs_leftx);
    valid &= evdev_init_parms(&devices[dev], &(devices[dev].yParms), devices[dev].map->abs_lefty);
    valid &= evdev_init_parms(&devices[dev], &(devices[dev].zParms), devices[dev].map->abs_lefttrigger);
    valid &= evdev_init_parms(&devices[dev], &(devices[dev].rxParms), devices[dev].map->abs_rightx);
    valid &= evdev_init_parms(&devices[dev], &(devices[dev].ryParms), devices[dev].map->abs_righty);
    valid &= evdev_init_parms(&devices[dev], &(devices[dev].rzParms), devices[dev].map->abs_righttrigger);
    valid &= evdev_init_parms(&devices[dev], &(devices[dev].leftParms), devices[dev].map->abs_dpleft);
    valid &= evdev_init_parms(&devices[dev], &(devices[dev].rightParms), devices[dev].map->abs_dpright);
    valid &= evdev_init_parms(&devices[dev], &(devices[dev].upParms), devices[dev].map->abs_dpup);
    valid &= evdev_init_parms(&devices[dev], &(devices[dev].downParms), devices[dev].map->abs_dpdown);
    if (!valid)
      fprintf(stderr, "Mapping for %s (%s) on %s is incorrect\n", name, str_guid, device);
  }

  if (grabbingDevices && (is_keyboard || is_mouse || is_touchscreen)) {
    if (ioctl(fd, EVIOCGRAB, 1) < 0) {
      fprintf(stderr, "EVIOCGRAB failed with error %d\n", errno);
    }
  }

  loop_add_fd(devices[dev].fd, &evdev_handle, POLLIN);
}

static void evdev_map_key(char* keyName, short* key) {
  printf("Press %s\n", keyName);
  currentKey = key;
  currentHat = NULL;
  currentAbs = NULL;
  *key = -1;
  loop_main();

  usleep(250000);
  evdev_drain();
}

static void evdev_map_abs(char* keyName, short* abs, bool* reverse) {
  printf("Move %s\n", keyName);
  currentKey = NULL;
  currentHat = NULL;
  currentAbs = abs;
  currentReverse = reverse;
  *abs = -1;
  loop_main();

  usleep(250000);
  evdev_drain();
}

static void evdev_map_hatkey(char* keyName, short* hat, short* hat_dir, short* key) {
  printf("Press %s\n", keyName);
  currentKey = key;
  currentHat = hat;
  currentHatDir = hat_dir;
  currentAbs = NULL;
  *key = -1;
  *hat = -1;
  *hat_dir = -1;
  *currentReverse = false;
  loop_main();

  usleep(250000);
  evdev_drain();
}

static void evdev_map_abskey(char* keyName, short* abs, short* key, bool* reverse) {
  printf("Press %s\n", keyName);
  currentKey = key;
  currentHat = NULL;
  currentAbs = abs;
  currentReverse = reverse;
  *key = -1;
  *abs = -1;
  *currentReverse = false;
  loop_main();

  usleep(250000);
  evdev_drain();
}

void evdev_map(char* device) {
  int fd = open(device, O_RDONLY|O_NONBLOCK);
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

  struct mapping map;
  strncpy(map.name, libevdev_get_name(evdev), sizeof(map.name));
  strncpy(map.guid, str_guid, sizeof(map.guid));

  libevdev_free(evdev);
  close(fd);

  handler = evdev_handle_mapping_event;

  evdev_map_abs("Left Stick Right", &(map.abs_leftx), &(map.reverse_leftx));
  evdev_map_abs("Left Stick Up", &(map.abs_lefty), &(map.reverse_lefty));
  evdev_map_key("Left Stick Button", &(map.btn_leftstick));

  evdev_map_abs("Right Stick Right", &(map.abs_rightx), &(map.reverse_rightx));
  evdev_map_abs("Right Stick Up", &(map.abs_righty), &(map.reverse_righty));
  evdev_map_key("Right Stick Button", &(map.btn_rightstick));

  evdev_map_hatkey("D-Pad Right", &(map.hat_dpright), &(map.hat_dir_dpright), &(map.btn_dpright));
  evdev_map_hatkey("D-Pad Left", &(map.hat_dpleft), &(map.hat_dir_dpleft), &(map.btn_dpleft));
  evdev_map_hatkey("D-Pad Up", &(map.hat_dpup), &(map.hat_dir_dpup), &(map.btn_dpup));
  evdev_map_hatkey("D-Pad Down", &(map.hat_dpdown), &(map.hat_dir_dpdown), &(map.btn_dpdown));

  evdev_map_key("Button X (1)", &(map.btn_x));
  evdev_map_key("Button A (2)", &(map.btn_a));
  evdev_map_key("Button B (3)", &(map.btn_b));
  evdev_map_key("Button Y (4)", &(map.btn_y));
  evdev_map_key("Back Button", &(map.btn_back));
  evdev_map_key("Start Button", &(map.btn_start));
  evdev_map_key("Special Button", &(map.btn_guide));

  bool ignored;
  evdev_map_abskey("Left Trigger", &(map.abs_lefttrigger), &(map.btn_lefttrigger), &ignored);
  evdev_map_abskey("Right Trigger", &(map.abs_righttrigger), &(map.btn_righttrigger), &ignored);

  evdev_map_key("Left Bumper", &(map.btn_leftshoulder));
  evdev_map_key("Right Bumper", &(map.btn_rightshoulder));
  mapping_print(&map);
}

void evdev_start() {
  // After grabbing, the only way to quit via the keyboard
  // is via the special key combo that the input handling
  // code looks for. For this reason, we wait to grab until
  // we're ready to take input events. Ctrl+C works up until
  // this point.
  for (int i = 0; i < numDevices; i++) {
    if ((devices[i].is_keyboard || devices[i].is_mouse || devices[i].is_touchscreen) && ioctl(devices[i].fd, EVIOCGRAB, 1) < 0) {
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

void evdev_init(bool mouse_emulation_enabled) {
  handler = evdev_handle_event;
  mouseEmulationEnabled = mouse_emulation_enabled;
}

static struct input_device* evdev_get_input_device(unsigned short controller_id) {
  for (int i=0; i<numDevices; i++)
    if (devices[i].controllerId == controller_id)
      return &devices[i];

  return NULL;
}

void evdev_rumble(unsigned short controller_id, unsigned short low_freq_motor, unsigned short high_freq_motor) {
  struct input_device* device = evdev_get_input_device(controller_id);
  if (!device)
    return;

  if (device->haptic_effect_id >= 0) {
    ioctl(device->fd, EVIOCRMFF, device->haptic_effect_id);
    device->haptic_effect_id = -1;
  }

  if (low_freq_motor == 0 && high_freq_motor == 0)
    return;

  struct ff_effect effect = {0};
  effect.type = FF_RUMBLE;
  effect.id = -1;
  effect.replay.length = USHRT_MAX;
  effect.u.rumble.strong_magnitude = low_freq_motor;
  effect.u.rumble.weak_magnitude = high_freq_motor;
  if (ioctl(device->fd, EVIOCSFF, &effect) == -1)
    return;

  struct input_event event = {0};
  event.type = EV_FF;
  event.code = effect.id;
  event.value = 1;
  write(device->fd, (const void*) &event, sizeof(event));
  device->haptic_effect_id = effect.id;
}
