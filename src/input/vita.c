/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2016 Ilya Zhuravlev, Sunguk Lee, Vasyl Horbachenko
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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <ctype.h>

#include "../graphics.h"
#include "../config.h"
#include "../connection.h"
#include "vita.h"
#include "mapping.h"

#include <Limelight.h>

#include <psp2/net/net.h>
#include <psp2/sysmodule.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/threadmgr.h>

#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/rtc.h>

#define WIDTH 960
#define HEIGHT 544

struct mapping map = {0};

typedef struct input_data {
    short button;
    short lx;
    short ly;
    short rx;
    short ry;
    char  lt;
    char  rt;
} input_data;

typedef struct Point {
  short x;
  short y;
} Point;

typedef struct Section {
  Point left;
  Point right;
} Section;

typedef struct TouchData {
  short button;
  short finger;
  Point points[4];
} TouchData;

#define lerp(value, from_max, to_max) ((((value*10) * (to_max*10))/(from_max*10))/10)

#define MOUSE_ACTION_DELAY 100000 // 100ms

inline bool mouse_click(short finger_count, bool press) {
  int mode;

  if (press) {
    mode = BUTTON_ACTION_PRESS;
  } else {
    mode = BUTTON_ACTION_RELEASE;
  }

  switch (finger_count) {
    case 1:
      LiSendMouseButtonEvent(mode, BUTTON_LEFT);
      return true;
    case 2:
      LiSendMouseButtonEvent(mode, BUTTON_RIGHT);
      return true;
  }
  return false;
}

inline void move_mouse(TouchData old, TouchData cur) {
  int delta_x = (cur.points[0].x - old.points[0].x) / 2;
  int delta_y = (cur.points[0].y - old.points[0].y) / 2;

  if (!delta_x && !delta_y) {
    return;
  }
  LiSendMouseMoveEvent(delta_x, delta_y);
}

inline void move_wheel(TouchData old, TouchData cur) {
  int old_y = (old.points[0].y + old.points[1].y) / 2;
  int cur_y = (cur.points[0].y + cur.points[1].y) / 2;
  int delta_y = (cur_y - old_y) / 2;
  if (!delta_y) {
    return;
  }
  LiSendScrollEvent(delta_y);
}


SceCtrlData pad, pad_old;
TouchData touch;
TouchData touch_old, swipe;
SceTouchData front, back;

int front_state = NO_TOUCH_ACTION;
short finger_count = 0;
SceRtcTick current, until;


static int special_status;

input_data curr, old;
int controller_port;

// TODO config
static int VERTICAL;
static int HORIZONTAL;

#define IN_SECTION(SECTION, X, Y) \
    ((SECTION).left.x <= (X) && (X) <= (SECTION).right.x && \
     (SECTION).left.y <= (Y) && (Y) <= (SECTION).right.y)

// TODO sections
Section BACK_SECTIONS[4];
Section FRONT_SECTIONS[4];

inline uint8_t read_backscreen() {
  for (int i = 0; i < back.reportNum; i++) {
    int x = lerp(back.report[i].x, 1919, WIDTH);
    int y = lerp(back.report[i].y, 1087, HEIGHT);

    if ((touch.button & TOUCHSEC_NORTHWEST) == 0) {
      if (IN_SECTION(BACK_SECTIONS[0], x, y)) {
        touch.button |= TOUCHSEC_NORTHWEST;
        continue;
      }
    }

    if ((touch.button & TOUCHSEC_NORTHEAST) == 0) {
      if (IN_SECTION(BACK_SECTIONS[1], x, y)) {
        touch.button |= TOUCHSEC_NORTHEAST;
        continue;
      }
    }

    if ((touch.button & TOUCHSEC_SOUTHWEST) == 0) {
      if (IN_SECTION(BACK_SECTIONS[2], x, y)) {
        touch.button |= TOUCHSEC_SOUTHWEST;
        continue;
      }
    }

    if ((touch.button & TOUCHSEC_SOUTHEAST) == 0) {
      if (IN_SECTION(BACK_SECTIONS[3], x, y)) {
        touch.button |= TOUCHSEC_SOUTHEAST;
        continue;
      }
    }
  }
  return 0;
}


inline uint8_t read_frontscreen() {
  for (int i = 0; i < front.reportNum; i++) {
    int x = lerp(front.report[i].x, 1919, WIDTH);
    int y = lerp(front.report[i].y, 1087, HEIGHT);

    if ((touch.button & TOUCHSEC_SPECIAL_NW) == 0) {
      if (IN_SECTION(FRONT_SECTIONS[0], x, y)) {
        touch.button |= TOUCHSEC_SPECIAL_NW;
        continue;
      }
    }

    if ((touch.button & TOUCHSEC_SPECIAL_NE) == 0) {
      if (IN_SECTION(FRONT_SECTIONS[1], x, y)) {
        touch.button |= TOUCHSEC_SPECIAL_NE;
        continue;
      }
    }

    if ((touch.button & TOUCHSEC_SPECIAL_SW) == 0) {
      if (IN_SECTION(FRONT_SECTIONS[2], x, y)) {
        touch.button |= TOUCHSEC_SPECIAL_SW;
        continue;
      }
    }

    if ((touch.button & TOUCHSEC_SPECIAL_SE) == 0) {
      if (IN_SECTION(FRONT_SECTIONS[3], x, y)) {
        touch.button |= TOUCHSEC_SPECIAL_SE;
        continue;
      }
    }

    // FIXME if touch same section using multiple finger, they can count finger
    touch.points[touch.finger].x = x;
    touch.points[touch.finger].y = y;
    touch.finger += 1;
  }
  return 0;
}

inline uint32_t is_pressed(uint32_t defined) {
  uint32_t dev_type = defined & INPUT_TYPE_MASK;
  uint32_t dev_val  = defined & INPUT_VALUE_MASK;

  switch(dev_type) {
    case INPUT_TYPE_GAMEPAD:
      return pad.buttons & dev_val;
    case INPUT_TYPE_TOUCHSCREEN:
      return touch.button & dev_val;
  }
  return 0;
}

inline uint32_t is_old_pressed(uint32_t defined) {
  uint32_t dev_type = defined & INPUT_TYPE_MASK;
  uint32_t dev_val  = defined & INPUT_VALUE_MASK;

  switch(dev_type) {
    case INPUT_TYPE_GAMEPAD:
      return pad_old.buttons & dev_val;
    case INPUT_TYPE_TOUCHSCREEN:
      return touch_old.button & dev_val;
  }
  return 0;
}

inline short read_analog(uint32_t defined) {
  uint32_t dev_type = defined & INPUT_TYPE_MASK;
  uint32_t dev_val  = defined & INPUT_VALUE_MASK;

  if (dev_type == INPUT_TYPE_ANALOG) {
    int v;
    switch(dev_val) {
      case LEFTX:
        v = pad.lx;
        break;
      case LEFTY:
        v = pad.ly;
        break;
      case RIGHTX:
        v = pad.rx;
        break;
      case RIGHTY:
        v = pad.ry;
        break;
      case LEFT_TRIGGER:
        return pad.lt;
      case RIGHT_TRIGGER:
        return pad.rt;
      default:
        return 0;
    }
    v = v * 256 - (1 << 15) + 128;
    return (short)(v);
  }
  return is_pressed(defined) ? 0xff : 0;
}

inline void special(uint32_t defined, uint32_t pressed, uint32_t old_pressed) {
  uint32_t dev_type = defined & INPUT_TYPE_MASK;
  uint32_t dev_val  = defined & INPUT_VALUE_MASK;

  if (pressed) {
    switch(dev_type) {
      case INPUT_TYPE_SPECIAL:
        if (dev_val == INPUT_SPECIAL_KEY_PAUSE) {
          connection_minimize();
          return;
        }
      case INPUT_TYPE_GAMEPAD:
        curr.button |= dev_val;
        return;
      case INPUT_TYPE_ANALOG:
        switch(dev_val) {
          case LEFT_TRIGGER:
            curr.lt = 0xff;
            return;
          case RIGHT_TRIGGER:
            curr.rt = 0xff;
            return;
        }
        return;
      case INPUT_TYPE_MOUSE:
        if (!old_pressed) {
          LiSendMouseButtonEvent(BUTTON_ACTION_PRESS, dev_val);
        }
        return;
      case INPUT_TYPE_KEYBOARD:
       if (!old_pressed) {
          LiSendKeyboardEvent(dev_val, KEY_ACTION_DOWN, 0);
       }
       return;
    }
  } else {
    // released
    switch(dev_type) {
      case INPUT_TYPE_MOUSE:
        if (old_pressed) {
          LiSendMouseButtonEvent(BUTTON_ACTION_RELEASE, dev_val);
        }
        return;
      case INPUT_TYPE_KEYBOARD:
        if (old_pressed) {
          LiSendKeyboardEvent(dev_val, KEY_ACTION_UP, 0);
        }
        return;
    }
  }

}

inline void vitainput_process(void) {
  memset(&pad, 0, sizeof(pad));
  memset(&touch, 0, sizeof(TouchData));
  memset(&curr, 0, sizeof(input_data));

  sceCtrlSetSamplingModeExt(SCE_CTRL_MODE_ANALOG_WIDE);
  sceCtrlReadBufferPositiveExt2(controller_port, &pad, 1);

  sceTouchPeek(SCE_TOUCH_PORT_FRONT, &front, 1);
  sceTouchPeek(SCE_TOUCH_PORT_BACK, &back, 1);
  read_frontscreen();
  read_backscreen();

  sceRtcGetCurrentTick(&current);

  // buttons
  curr.button |= is_pressed(map.btn_dpad_up)    ? UP_FLAG     : 0;
  curr.button |= is_pressed(map.btn_dpad_left)  ? LEFT_FLAG   : 0;
  curr.button |= is_pressed(map.btn_dpad_down)  ? DOWN_FLAG   : 0;
  curr.button |= is_pressed(map.btn_dpad_right) ? RIGHT_FLAG  : 0;
  curr.button |= is_pressed(map.btn_start)      ? PLAY_FLAG   : 0;
  curr.button |= is_pressed(map.btn_select)     ? BACK_FLAG   : 0;
  curr.button |= is_pressed(map.btn_north)      ? Y_FLAG      : 0;
  curr.button |= is_pressed(map.btn_east)       ? B_FLAG      : 0;
  curr.button |= is_pressed(map.btn_south)      ? A_FLAG      : 0;
  curr.button |= is_pressed(map.btn_west)       ? X_FLAG      : 0;
  curr.button |= is_pressed(map.btn_thumbl)     ? LB_FLAG     : 0; // l1
  curr.button |= is_pressed(map.btn_thumbr)     ? RB_FLAG     : 0; // r1
  curr.button |= is_pressed(map.btn_tl2)        ? LS_CLK_FLAG : 0; // l3
  curr.button |= is_pressed(map.btn_tr2)        ? RS_CLK_FLAG : 0; // r3

  // analogs
  curr.lt = read_analog(map.btn_tl); // l2
  curr.rt = read_analog(map.btn_tr); // r2
  curr.lx = read_analog(map.abs_x);
  curr.ly = read_analog(map.abs_y);
  curr.rx = read_analog(map.abs_rx);
  curr.ry = read_analog(map.abs_ry);

  // special touchscreen buttons
  special(config.special_keys.nw,
          is_pressed(INPUT_TYPE_TOUCHSCREEN | TOUCHSEC_SPECIAL_NW),
          is_old_pressed(INPUT_TYPE_TOUCHSCREEN | TOUCHSEC_SPECIAL_NW));
  special(config.special_keys.ne,
          is_pressed(INPUT_TYPE_TOUCHSCREEN | TOUCHSEC_SPECIAL_NE),
          is_old_pressed(INPUT_TYPE_TOUCHSCREEN | TOUCHSEC_SPECIAL_NE));
  special(config.special_keys.sw,
          is_pressed(INPUT_TYPE_TOUCHSCREEN | TOUCHSEC_SPECIAL_SW),
          is_old_pressed(INPUT_TYPE_TOUCHSCREEN | TOUCHSEC_SPECIAL_SW));
  special(config.special_keys.se,
          is_pressed(INPUT_TYPE_TOUCHSCREEN | TOUCHSEC_SPECIAL_SE),
          is_old_pressed(INPUT_TYPE_TOUCHSCREEN | TOUCHSEC_SPECIAL_SE));

  // mouse
  switch (front_state) {
    case NO_TOUCH_ACTION:
      if (touch.finger > 0) {
        front_state = ON_SCREEN_TOUCH;
        finger_count = touch.finger;
        sceRtcTickAddMicroseconds(&until, &current, MOUSE_ACTION_DELAY);
      }
      break;
    case ON_SCREEN_TOUCH:
      if (sceRtcCompareTick(&current, &until) < 0) {
        if (touch.finger < finger_count) {
          // TAP
          if (mouse_click(finger_count, true)) {
            front_state = SCREEN_TAP;
            sceRtcTickAddMicroseconds(&until, &current, MOUSE_ACTION_DELAY);
          } else {
            front_state = NO_TOUCH_ACTION;
          }
        } else if (touch.finger > finger_count) {
          // finger count changed
          finger_count = touch.finger;
        }
      } else {
        front_state = SWIPE_START;
      }
      break;
    case SCREEN_TAP:
      if (sceRtcCompareTick(&current, &until) >= 0) {
        mouse_click(finger_count, false);

        front_state = NO_TOUCH_ACTION;
      }
      break;
    case SWIPE_START:
      memcpy(&swipe, &touch, sizeof(swipe));
      front_state = ON_SCREEN_SWIPE;
      break;
    case ON_SCREEN_SWIPE:
      if (touch.finger > 0) {
        switch (touch.finger) {
          case 1:
            move_mouse(swipe, touch);
            break;
          case 2:
            move_wheel(swipe, touch);
            break;
        }
        memcpy(&swipe, &touch, sizeof(swipe));
      } else {
        front_state = NO_TOUCH_ACTION;
      }
      break;
  }

  if (memcmp(&curr, &old, sizeof(input_data)) != 0) {
    LiSendControllerEvent(curr.button, curr.lt, curr.rt,
                          curr.lx, -1 * curr.ly, curr.rx, -1 * curr.ry);
    memcpy(&old, &curr, sizeof(input_data));
    memcpy(&pad_old, &pad, sizeof(SceCtrlData));
  }
  if (memcmp(&touch, &touch_old, sizeof(TouchData)) != 0) {
    memcpy(&touch_old, &touch, sizeof(TouchData));
  }
}

static uint8_t active_input_thread = 0;

int vitainput_thread(SceSize args, void *argp) {
  while (1) {
    if (active_input_thread) {
      vitainput_process();
    }

    sceKernelDelayThread(5000); // 5 ms
  }

  return 0;
}

bool vitainput_init() {
  sceCtrlSetSamplingModeExt(SCE_CTRL_MODE_ANALOG_WIDE);
  sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
  sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, SCE_TOUCH_SAMPLING_STATE_START);

  SceUID thid = sceKernelCreateThread("vitainput_thread", vitainput_thread, 0, 0x40000, 0, 0, NULL);
  if (thid >= 0) {
    sceKernelStartThread(thid, 0, NULL);
    return true;
  }

  return false;
}

void vitainput_config(CONFIGURATION config) {
  map.abs_x           = LEFTX               | INPUT_TYPE_ANALOG;
  map.abs_y           = LEFTY               | INPUT_TYPE_ANALOG;
  map.abs_rx          = RIGHTX              | INPUT_TYPE_ANALOG;
  map.abs_ry          = RIGHTY              | INPUT_TYPE_ANALOG;

  map.btn_dpad_up     = SCE_CTRL_UP         | INPUT_TYPE_GAMEPAD;
  map.btn_dpad_down   = SCE_CTRL_DOWN       | INPUT_TYPE_GAMEPAD;
  map.btn_dpad_left   = SCE_CTRL_LEFT       | INPUT_TYPE_GAMEPAD;
  map.btn_dpad_right  = SCE_CTRL_RIGHT      | INPUT_TYPE_GAMEPAD;
  map.btn_south       = SCE_CTRL_CROSS      | INPUT_TYPE_GAMEPAD;
  map.btn_east        = SCE_CTRL_CIRCLE     | INPUT_TYPE_GAMEPAD;
  map.btn_north       = SCE_CTRL_TRIANGLE   | INPUT_TYPE_GAMEPAD;
  map.btn_west        = SCE_CTRL_SQUARE     | INPUT_TYPE_GAMEPAD;

  map.btn_select      = SCE_CTRL_SELECT     | INPUT_TYPE_GAMEPAD;
  map.btn_start       = SCE_CTRL_START      | INPUT_TYPE_GAMEPAD;

  map.btn_thumbl      = SCE_CTRL_L1         | INPUT_TYPE_GAMEPAD;
  map.btn_thumbr      = SCE_CTRL_R1         | INPUT_TYPE_GAMEPAD;

  if (config.model == SCE_KERNEL_MODEL_VITATV) {
    map.btn_tl        = LEFT_TRIGGER        | INPUT_TYPE_ANALOG;
    map.btn_tr        = RIGHT_TRIGGER       | INPUT_TYPE_ANALOG;
    map.btn_tl2       = SCE_CTRL_L3         | INPUT_TYPE_GAMEPAD;
    map.btn_tr2       = SCE_CTRL_R3         | INPUT_TYPE_GAMEPAD;
  } else {
    map.btn_tl        = TOUCHSEC_NORTHWEST  | INPUT_TYPE_TOUCHSCREEN;
    map.btn_tr        = TOUCHSEC_NORTHEAST  | INPUT_TYPE_TOUCHSCREEN;
    map.btn_tl2       = TOUCHSEC_SOUTHWEST  | INPUT_TYPE_TOUCHSCREEN;
    map.btn_tr2       = TOUCHSEC_SOUTHEAST  | INPUT_TYPE_TOUCHSCREEN;
  }

  if (config.mapping) {
    char config_path[256];
    sprintf(config_path, "ux0:data/moonlight/%s", config.mapping);
    printf("Loading mapping at %s\n", config_path);
    mapping_load(config_path, &map);
  }

  controller_port = config.model == SCE_KERNEL_MODEL_VITATV ? 1 : 0;

  VERTICAL   = (WIDTH - config.back_deadzone.left - config.back_deadzone.right) / 2
             + config.back_deadzone.left;
  HORIZONTAL = (HEIGHT - config.back_deadzone.top - config.back_deadzone.bottom) / 2
             + config.back_deadzone.top;

  BACK_SECTIONS[0].left.x  = config.back_deadzone.left;
  BACK_SECTIONS[0].left.y  = config.back_deadzone.top;
  BACK_SECTIONS[0].right.x = VERTICAL;
  BACK_SECTIONS[0].right.y = HORIZONTAL;

  BACK_SECTIONS[1].left.x  = VERTICAL;
  BACK_SECTIONS[1].left.y  = config.back_deadzone.top;
  BACK_SECTIONS[1].right.x = WIDTH - config.back_deadzone.right;
  BACK_SECTIONS[1].right.y = HORIZONTAL;

  BACK_SECTIONS[2].left.x  = config.back_deadzone.left;
  BACK_SECTIONS[2].left.y  = HORIZONTAL;
  BACK_SECTIONS[2].right.x = VERTICAL;
  BACK_SECTIONS[2].right.y = HEIGHT - config.back_deadzone.bottom;

  BACK_SECTIONS[3].left.x  = VERTICAL;
  BACK_SECTIONS[3].left.y  = HORIZONTAL;
  BACK_SECTIONS[3].right.x = WIDTH - config.back_deadzone.right;
  BACK_SECTIONS[3].right.y = HEIGHT - config.back_deadzone.bottom;

  FRONT_SECTIONS[0].left.x  = config.special_keys.offset;
  FRONT_SECTIONS[0].left.y  = config.special_keys.offset;
  FRONT_SECTIONS[0].right.x = config.special_keys.offset + config.special_keys.size;
  FRONT_SECTIONS[0].right.y = config.special_keys.offset + config.special_keys.size;

  FRONT_SECTIONS[1].left.x  = WIDTH - config.special_keys.offset - config.special_keys.size;
  FRONT_SECTIONS[1].left.y  = config.special_keys.offset;
  FRONT_SECTIONS[1].right.x = WIDTH - config.special_keys.offset;
  FRONT_SECTIONS[1].right.y = config.special_keys.offset + config.special_keys.size;

  FRONT_SECTIONS[2].left.x  = config.special_keys.offset;
  FRONT_SECTIONS[2].left.y  = HEIGHT - config.special_keys.offset - config.special_keys.size;
  FRONT_SECTIONS[2].right.x = config.special_keys.offset + config.special_keys.size;
  FRONT_SECTIONS[2].right.y = HEIGHT - config.special_keys.offset;

  FRONT_SECTIONS[3].left.x  = WIDTH - config.special_keys.offset - config.special_keys.size;
  FRONT_SECTIONS[3].left.y  = HEIGHT - config.special_keys.offset - config.special_keys.size;
  FRONT_SECTIONS[3].right.x = WIDTH - config.special_keys.offset;
  FRONT_SECTIONS[3].right.y = HEIGHT - config.special_keys.offset;
}

void vitainput_start(void) {
  active_input_thread = true;
}

void vitainput_stop(void) {
  active_input_thread = false;
}
