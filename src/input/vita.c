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
#include "vita.h"
#include "mapping.h"

#include <Limelight.h>

#include <psp2/net/net.h>
#include <psp2/sysmodule.h>
#include <psp2/kernel/sysmem.h>

#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/rtc.h>

#define WIDTH 960
#define HEIGHT 544

static struct mapping map = {0};

typedef struct input_data {
    short button;
    char left_trigger;
    char right_trigger;
    short lx;
    short ly;
    short rx;
    short ry;
} input_data;

#define lerp(value, from_max, to_max) ((((value*10) * (to_max*10))/(from_max*10))/10)

static int check_touch(SceTouchData scr, int lx, int ly, int rx, int ry) {
  for (int i = 0; i < scr.reportNum; i++) {
    int x = lerp(scr.report[i].x, 1919, WIDTH);
    int y = lerp(scr.report[i].y, 1087, HEIGHT);
    if (x < lx || x > rx || y < ly || y > ry) continue;
    return i;
  }
  return -1;
}

int check_touch_sector(SceTouchData scr, int section) {
  int vertical = (WIDTH - config.back_deadzone.left - config.back_deadzone.right) / 2 + config.back_deadzone.left,
      horizontal = (HEIGHT - config.back_deadzone.top - config.back_deadzone.bottom) / 2 + config.back_deadzone.top;

  int special_offset = config.special_keys.offset,
      special_size = config.special_keys.size;

  switch (section) {
    case TOUCHSEC_NORTHWEST:
      return check_touch(scr, config.back_deadzone.left, config.back_deadzone.top, vertical, horizontal);
    case TOUCHSEC_NORTHEAST:
      return check_touch(scr, vertical, config.back_deadzone.top, WIDTH - config.back_deadzone.right, horizontal);
    case TOUCHSEC_SOUTHWEST:
      return check_touch(scr, config.back_deadzone.left, horizontal, vertical, HEIGHT - config.back_deadzone.bottom);
    case TOUCHSEC_SOUTHEAST:
      return check_touch(scr, vertical, horizontal, WIDTH - config.back_deadzone.left, HEIGHT - config.back_deadzone.bottom);
    case TOUCHSEC_SPECIAL_SW:
      return check_touch(
          scr,
          special_offset,
          HEIGHT - special_size - special_offset,
          special_size + special_offset,
          HEIGHT - special_offset
          );
    case TOUCHSEC_SPECIAL_SE:
      return check_touch(
          scr,
          WIDTH - special_size - special_offset,
          HEIGHT - special_size - special_offset,
          WIDTH - special_offset,
          HEIGHT - special_offset
          );
    case TOUCHSEC_SPECIAL_NW:
      return check_touch(
          scr,
          special_offset,
          special_offset,
          special_offset + special_size,
          special_offset + special_size
          );
    case TOUCHSEC_SPECIAL_NE:
      return check_touch(
          scr,
          WIDTH - special_size - special_offset,
          special_offset,
          WIDTH - special_offset,
          special_offset + special_size
          );

    default:
      return false;
  }
}

#define MOUSE_ACTION_DELAY 100000 // 100ms

static bool mouse_click(SceTouchData screen, short finger_count, bool press) {
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

bool check_input(uint32_t identifier, SceCtrlData pad, SceTouchData screen) {
  uint32_t type = identifier & INPUT_TYPE_MASK;
  int value = identifier & INPUT_VALUE_MASK;

  switch (type) {
    case INPUT_TYPE_TOUCHSCREEN:
      if (value < TOUCHSEC_NORTHWEST || value > TOUCHSEC_SOUTHEAST) {
        return false;
      }
      return check_touch_sector(screen, value) != -1;
    case INPUT_TYPE_GAMEPAD:
      return pad.buttons & value;
  }
  return false;
}

static short pad_analog_value(uint32_t identifier, SceCtrlData pad, SceTouchData screen) {
  unsigned char value = 0;

  if ((identifier & INPUT_TYPE_MASK) != INPUT_TYPE_ANALOG) {
    return check_input(identifier, pad, screen) ? 0xff : 0;
  }

  switch (identifier & INPUT_VALUE_MASK) {
    case LEFT_TRIGGER:
      return pad.lt;
    case RIGHT_TRIGGER:
      return pad.rt;
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

static int special_input_config_code(short identifier) {
  switch (identifier) {
    case TOUCHSEC_SPECIAL_NW:
      return config.special_keys.nw;
    case TOUCHSEC_SPECIAL_NE:
      return config.special_keys.ne;
    case TOUCHSEC_SPECIAL_SW:
      return config.special_keys.sw;
    case TOUCHSEC_SPECIAL_SE:
      return config.special_keys.se;
    default:
      return 0;
  }
}

static bool special_input_status[4] = {0, 0, 0, 0};
static void special_input(SceTouchData screen, input_data *input) {
  for (int identifier = TOUCHSEC_SPECIAL_NW; identifier <= TOUCHSEC_SPECIAL_SE; identifier++) {
    int idx = identifier - TOUCHSEC_SPECIAL_NW;
    bool current_status = special_input_status[idx];
    unsigned int config_code = special_input_config_code(identifier);

    unsigned int type = config_code & INPUT_TYPE_MASK;
    unsigned int code = config_code & INPUT_VALUE_MASK;
    if (check_touch_sector(screen, identifier) != -1 && !current_status) {
      switch (type) {
        case INPUT_TYPE_SPECIAL:
          if (code == INPUT_SPECIAL_KEY_PAUSE) {
            connection_minimize();
          } break;
        case INPUT_TYPE_GAMEPAD:
          input->button |= code;
          break;
        case INPUT_TYPE_MOUSE:
          special_input_status[idx] = true;
          LiSendMouseButtonEvent(BUTTON_ACTION_PRESS, code);
          break;
        case INPUT_TYPE_ANALOG:
          switch (code) {
            case LEFT_TRIGGER:
              input->left_trigger = 0xff;
              break;
            case RIGHT_TRIGGER:
              input->right_trigger = 0xff;
              break;
          }
          break;
        case INPUT_TYPE_KEYBOARD:
          special_input_status[idx] = true;
          LiSendKeyboardEvent(code, KEY_ACTION_DOWN, 0);
          break;
      }
    } else if (check_touch_sector(screen, identifier) == -1 && current_status) {
      special_input_status[idx] = false;

      switch (type) {
        case INPUT_TYPE_MOUSE:
          LiSendMouseButtonEvent(BUTTON_ACTION_RELEASE, code);
          break;
        case INPUT_TYPE_KEYBOARD:
          LiSendKeyboardEvent(code, KEY_ACTION_UP, 0);
          break;
      }
    }
  }
}

#define INPUT_BUTTON(id, flag)  if (check_input((id), pad, back)) input.button |= (flag);
#define INPUT_ANALOG(id)        pad_analog_value((id), pad, back)

static SceCtrlData pad;
static SceTouchData front;
static SceTouchData front_old;
static SceTouchData back;

static int front_state = NO_TOUCH_ACTION;
static short finger_count = 0;
static SceRtcTick current, until;


static int special_status;

static input_data old;
static int controller_port;

void vitainput_process(void) {
  memset(&pad, 0, sizeof(pad));

  sceCtrlReadBufferPositiveExt2(controller_port, &pad, 1);

  sceTouchPeek(SCE_TOUCH_PORT_FRONT, &front, 1);
  sceTouchPeek(SCE_TOUCH_PORT_BACK, &back, 1);
  sceRtcGetCurrentTick(&current);

  input_data input = {0};

  // buttons
  INPUT_BUTTON(map.btn_dpad_up,     UP_FLAG);
  INPUT_BUTTON(map.btn_dpad_left,   LEFT_FLAG);
  INPUT_BUTTON(map.btn_dpad_down,   DOWN_FLAG);
  INPUT_BUTTON(map.btn_dpad_right,  RIGHT_FLAG);

  INPUT_BUTTON(map.btn_start,       PLAY_FLAG);
  INPUT_BUTTON(map.btn_select,      BACK_FLAG);

  INPUT_BUTTON(map.btn_north,       Y_FLAG);
  INPUT_BUTTON(map.btn_east,        B_FLAG);
  INPUT_BUTTON(map.btn_south,       A_FLAG);
  INPUT_BUTTON(map.btn_west,        X_FLAG);

  INPUT_BUTTON(map.btn_tl2,         LS_CLK_FLAG);
  INPUT_BUTTON(map.btn_tr2,         RS_CLK_FLAG);

  INPUT_BUTTON(map.btn_thumbl,      LB_FLAG);
  INPUT_BUTTON(map.btn_thumbr,      RB_FLAG);

  // ANALOG
  input.left_trigger  = INPUT_ANALOG(map.btn_tl);
  input.right_trigger = INPUT_ANALOG(map.btn_tr);

  input.lx            = INPUT_ANALOG(map.abs_x);
  input.ly            = INPUT_ANALOG(map.abs_y);
  input.rx            = INPUT_ANALOG(map.abs_rx);
  input.ry            = INPUT_ANALOG(map.abs_ry);

  // special touchscreen buttons
  special_input(front, &input);

  // remove touches for special actions
  for (int i = TOUCHSEC_SPECIAL_NW, touchIdx = -1; i <= TOUCHSEC_SPECIAL_SE; i++) {
    unsigned int config_code = special_input_config_code(i);
    if (config_code && (touchIdx = check_touch_sector(front, i)) != -1) {
      for (int n = 0, idx = 0; n < front.reportNum; n++) {
        if (n != touchIdx) {
          front.report[idx++] = front.report[n];
        }
      }

      front.reportNum--;
    }
  }

  // mouse
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
        if (front.reportNum < finger_count) {
          // TAP
          if (mouse_click(front, finger_count, true)) {
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
        mouse_click(front, finger_count, false);

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

  if (memcmp(&input, &old, sizeof(input_data)) != 0) {
    LiSendControllerEvent(input.button, input.left_trigger, input.right_trigger,
                          input.lx, -1 * input.ly, input.rx, -1 * input.ry);
    memcpy(&old, &input, sizeof(input_data));
  }
}

static uint8_t active_input_thread = 0;

int vitainput_thread(SceSize args, void *argp) {
  while (1) {
    if (active_input_thread) {
      vitainput_process();
    }

    sceKernelDelayThread(5000); // 5 ms

    /*
    SceRtcTick before, after;
    sceRtcGetCurrentTick(&before);
    sceKernelDelayThread(1 * 1000); // 1 ms
    sceRtcGetCurrentTick(&after);

    if (active_input_thread) {
      if (after.tick - before.tick > 150 * 1000) {
        connection_terminate();
      }
    } */
  }

  return 0;
}

bool vitainput_init() {
  sceCtrlSetSamplingModeExt(SCE_CTRL_MODE_ANALOG_WIDE);
  sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
  sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, SCE_TOUCH_SAMPLING_STATE_START);

  SceUID thid = sceKernelCreateThread("vitainput_thread", vitainput_thread, 0x10000100, 0x40000, 0, 0, NULL);
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
}

void vitainput_start(void) {
  active_input_thread = true;
}

void vitainput_stop(void) {
  active_input_thread = false;
}
