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
#include "mapping.h"

#include <Limelight.h>

#include <psp2/net/net.h>
#include <psp2/sysmodule.h>

#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/rtc.h>

enum {
  NO_TOUCH_ACTION = 0,
  ON_SCREEN_TOUCH,
  SCREEN_TAP,
  SWIPE_START,
  ON_SCREEN_SWIPE
} TouchScreenState;

enum {
  TOUCHSEC_NORTHWEST = 600,
  TOUCHSEC_NORTHEAST,
  TOUCHSEC_SOUTHWEST,
  TOUCHSEC_SOUTHEAST,
  TOUCHSEC_SPECIAL = 700
} TouchScreenSection;

enum {
  LEFTX,
  LEFTY,
  RIGHTX,
  RIGHTY
} PadSection;

static struct mapping map = {0};

#define lerp(value, from_max, to_max) ((((value*10) * (to_max*10))/(from_max*10))/10)

static bool check_touch(SceTouchData scr, int lx, int ly, int rx, int ry) {
  for (int i = 0; i < scr.reportNum; i++) {
    int x = lerp(scr.report[i].x, 1919, 960);
    int y = lerp(scr.report[i].y, 1087, 544);
    if (x < lx || x > rx || y < ly || y > ry) continue;
    return true;
  }
  return false;
}

static bool check_touch_sector(SceTouchData scr, int section) {
  int vertical = (960 - config.back_deadzone.left - config.back_deadzone.right) / 2 + config.back_deadzone.left,
      horizontal = (544 - config.back_deadzone.top - config.back_deadzone.bottom) / 2 + config.back_deadzone.top;
  switch (section) {
    case TOUCHSEC_NORTHWEST:
      return check_touch(scr, config.back_deadzone.left, config.back_deadzone.top, vertical, horizontal);
    case TOUCHSEC_NORTHEAST:
      return check_touch(scr, vertical, config.back_deadzone.top, 960 - config.back_deadzone.right, horizontal);
    case TOUCHSEC_SOUTHWEST:
      return check_touch(scr, config.back_deadzone.left, horizontal, vertical, 544 - config.back_deadzone.bottom);
    case TOUCHSEC_SOUTHEAST:
      return check_touch(scr, vertical, horizontal, 960 - config.back_deadzone.left, 544 - config.back_deadzone.bottom);
    case TOUCHSEC_SPECIAL:
      if (true) {
        int offset = 30;
        int size = 200;
        return check_touch(scr, offset, 544 - size - offset, size + offset, 544 - offset);
      }

    default:
      return false;
  }
}

#define MOUSE_ACTION_DELAY 100000 // 100ms

static bool mouse_click(short finger_count, bool press) {
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

static void move_mouse(SceTouchData old, SceTouchData cur) {
  int delta_x = (cur.report[0].x - old.report[0].x) / 2;
  int delta_y = (cur.report[0].y - old.report[0].y) / 2;

  if (!delta_x && !delta_y) {
    return;
  }
  LiSendMouseMoveEvent(delta_x, delta_y);
}

static void move_wheel(SceTouchData old, SceTouchData cur) {
  int old_y = (old.report[0].y + old.report[1].y) / 2;
  int cur_y = (cur.report[0].y + cur.report[1].y) / 2;
  int delta_y = (cur_y - old_y) / 2;
  if (!delta_y) {
    return;
  }
  LiSendScrollEvent(delta_y);
}

static short pad_value(SceCtrlData pad, int sec) {
  unsigned char value = 0;
  switch (sec) {
    case LEFTX:
      value = pad.lx;
      break;
    case RIGHTX:
      value = pad.rx;
      break;
    case LEFTY:
      value = pad.ly;
      break;
    case RIGHTY:
      value = pad.ry;
      break;
  }

  return (short) (value * 256 - (1 << 15) + 128);
}

bool check_input(short identifier, SceCtrlData pad, SceTouchData screen, SceTouchData front, SceTouchData back) {
  if (identifier >= TOUCHSEC_NORTHWEST && identifier < TOUCHSEC_SPECIAL) {
    return check_touch_sector(identifier == TOUCHSEC_SPECIAL ? front : screen, identifier);
  } else if (identifier >= TOUCHSEC_SPECIAL && identifier <= TOUCHSEC_SPECIAL) {
    return check_touch_sector(front, identifier);
  } else {
    identifier = identifier + 1;
    return pad.buttons & identifier;
  }
}

#define CHECK_INPUT(id) check_input((id), pad, buttons_screen, front, back)
#define INPUT(id, flag) if (check_input((id), pad, buttons_screen, front, back)) btn |= (flag);

bool vitainput_init(CONFIGURATION conf) {
  map.abs_x = 0;
  map.abs_y = 1;
  map.abs_rx = 2;
  map.abs_ry = 3;

  map.btn_south = SCE_CTRL_CROSS - 1;
  map.btn_east = SCE_CTRL_CIRCLE - 1;
  map.btn_north = SCE_CTRL_TRIANGLE - 1;
  map.btn_west = SCE_CTRL_SQUARE - 1;
  map.btn_select = SCE_CTRL_SELECT - 1;
  map.btn_start = SCE_CTRL_START - 1;
  map.btn_thumbl = SCE_CTRL_LTRIGGER - 1;
  map.btn_thumbr = SCE_CTRL_RTRIGGER - 1;
  map.btn_dpad_up = SCE_CTRL_UP - 1;
  map.btn_dpad_down = SCE_CTRL_DOWN - 1;
  map.btn_dpad_left = SCE_CTRL_LEFT - 1;
  map.btn_dpad_right = SCE_CTRL_RIGHT - 1;

  map.btn_tl = TOUCHSEC_NORTHWEST;
  map.btn_tr = TOUCHSEC_NORTHEAST;
  map.btn_tl2 = TOUCHSEC_SOUTHWEST;
  map.btn_tr2 = TOUCHSEC_SOUTHEAST;
  map.btn_mode = TOUCHSEC_SPECIAL;

  config = conf;

  if (config.mapping) {
    char config_path[256];
    sprintf(config_path, "ux0:data/moonlight/%s", config.mapping);
    mapping_load(config_path, &map);
  }

  return true;
}

void vitainput_loop(void) {
  sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);
  sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
  sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, SCE_TOUCH_SAMPLING_STATE_START);

  SceCtrlData pad;
  SceTouchData front;
  SceTouchData front_old;
  SceTouchData back;

  int front_state = NO_TOUCH_ACTION;
  short finger_count = 0;
  SceRtcTick current, until;

  while (1) {
    memset(&pad, 0, sizeof(pad));

    sceCtrlPeekBufferPositive(0, &pad, 1);
    sceTouchPeek(SCE_TOUCH_PORT_FRONT, &front, 1);
    sceTouchPeek(SCE_TOUCH_PORT_BACK, &back, 1);
    sceRtcGetCurrentTick(&current);

    if (config.fronttouchscreen_buttons == false) {
      switch (front_state) {
        case NO_TOUCH_ACTION:
          if (front.reportNum > 0) {
            front_state = ON_SCREEN_TOUCH;
            finger_count = front.reportNum;
            sceRtcTickAddMicroseconds(&until, &current, MOUSE_ACTION_DELAY);
          }
          break;
        case ON_SCREEN_TOUCH:
          if (sceRtcCompareTick(&current, &until) < 0) {
            if (front.reportNum < finger_count && !check_touch_sector(front, TOUCHSEC_SPECIAL)) {
              // TAP
              if (mouse_click(finger_count, true)) {
                front_state = SCREEN_TAP;
                sceRtcTickAddMicroseconds(&until, &current, MOUSE_ACTION_DELAY);
              } else {
                front_state = NO_TOUCH_ACTION;
              }
            } else if (front.reportNum > finger_count) {
              // finger count changed
              finger_count = front.reportNum;
            }
          } else {
            front_state = SWIPE_START;
          }
          break;
        case SCREEN_TAP:
          if (sceRtcCompareTick(&current, &until) >= 0) {
            if (!check_touch_sector(front, TOUCHSEC_SPECIAL)) {
              mouse_click(finger_count, false);
            }

            front_state = NO_TOUCH_ACTION;
          }
          break;
        case SWIPE_START:
          memcpy(&front_old, &front, sizeof(front_old));
          front_state = ON_SCREEN_SWIPE;
          break;
        case ON_SCREEN_SWIPE:
          if (front.reportNum > 0) {
            switch (front.reportNum) {
              case 1:
                move_mouse(front_old, front);
                break;
              case 2:
                move_wheel(front_old, front);
                break;
            }
            memcpy(&front_old, &front, sizeof(front_old));
          } else {
            front_state = NO_TOUCH_ACTION;
          }
          break;
      }
    }

    SceTouchData buttons_screen = config.fronttouchscreen_buttons ? front : back;

    short btn = 0;
    INPUT(map.btn_dpad_up, UP_FLAG);
    INPUT(map.btn_dpad_up, UP_FLAG);
    INPUT(map.btn_dpad_left, LEFT_FLAG);
    INPUT(map.btn_dpad_down, DOWN_FLAG);
    INPUT(map.btn_dpad_right, RIGHT_FLAG);

    INPUT(map.btn_start, PLAY_FLAG);
    INPUT(map.btn_select, BACK_FLAG);

    INPUT(map.btn_north, Y_FLAG);
    INPUT(map.btn_east, B_FLAG);
    INPUT(map.btn_south, A_FLAG);
    INPUT(map.btn_west, X_FLAG);

    INPUT(map.btn_tl2, LS_CLK_FLAG);
    INPUT(map.btn_tr2, RS_CLK_FLAG);

    INPUT(map.btn_thumbl, LB_FLAG);
    INPUT(map.btn_thumbr, RB_FLAG);

    INPUT(map.btn_mode, SPECIAL_FLAG);

    // TRIGGERS
    char left_trigger_value = CHECK_INPUT(map.btn_tl) ? 0xff : 0;
    char right_trigger_value = CHECK_INPUT(map.btn_tr) ? 0xff : 0;

    short lx = pad_value(pad, map.abs_x),
          ly = pad_value(pad, map.abs_y),
          rx = pad_value(pad, map.abs_rx),
          ry = pad_value(pad, map.abs_ry);

    LiSendControllerEvent(btn, left_trigger_value, right_trigger_value, lx, -ly, rx, -ry);

    SceRtcTick before, after;
    sceRtcGetCurrentTick(&before);
    sceKernelDelayThread(1 * 1000); // 1 ms
    sceRtcGetCurrentTick(&after);

    // TODO: less hacky way?
    if (after.tick - before.tick > 50 * 1000) {
      return;
    }
  }
}
