#include "ui_settings.h"

#include "guilib.h"
#include "ime.h"

#include "../config.h"
#include "../input/vita.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>

#include <psp2/ctrl.h>
#include <psp2/rtc.h>
#include <psp2/touch.h>
#include <vita2d.h>
#include <Limelight.h>

static unsigned int settings_special_codes[] = {0,
  // special
  INPUT_TYPE_DEF_NAME | INPUT_TYPE_SPECIAL,
  INPUT_SPECIAL_KEY_PAUSE | INPUT_TYPE_SPECIAL,
  // gamepad
  INPUT_TYPE_DEF_NAME | INPUT_TYPE_GAMEPAD,
  SPECIAL_FLAG | INPUT_TYPE_GAMEPAD,
  LB_FLAG | INPUT_TYPE_GAMEPAD,
  RB_FLAG | INPUT_TYPE_GAMEPAD,
  LS_CLK_FLAG | INPUT_TYPE_GAMEPAD,
  RS_CLK_FLAG | INPUT_TYPE_GAMEPAD,
  LEFT_TRIGGER | INPUT_TYPE_ANALOG,
  RIGHT_TRIGGER | INPUT_TYPE_ANALOG,
  // mouse
  INPUT_TYPE_DEF_NAME | INPUT_TYPE_MOUSE,
  BUTTON_LEFT | INPUT_TYPE_MOUSE,
  BUTTON_RIGHT | INPUT_TYPE_MOUSE,
  BUTTON_MIDDLE | INPUT_TYPE_MOUSE,
  BUTTON_X1 | INPUT_TYPE_MOUSE,
  BUTTON_X2 | INPUT_TYPE_MOUSE,
  // keyboard
  INPUT_TYPE_DEF_NAME | INPUT_TYPE_KEYBOARD,
  27,    73,  77,  9,
  112,  113,  114,  115,  116,  117,  118,  119, 120,   121,   122,   123
};

static char *settings_special_names[] = {"None",
  // special
  "Special inputs",
  "Pause stream",
  // gamepad
  "Gamepad buttons",
  "Special (XBox button)",
  "LB", "RB", "LS", "RS", "LT", "RT",
  // mouse
  "Mouse buttons",
  "Left",
  "Right",
  "Middle(wheel)",
  "X1(4th)",
  "X2(5th)",
  // keyboard
  "Keyboard input codes",
  "Esc",
  "I", "M", "Tab",
  "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12"
};

static bool settings_loop_setup = 1;

/*
 * Deadzone
 */

#define lerp(value, from_max, to_max) ((((value*10) * (to_max*10))/(from_max*10))/10)
static void deadzone_draw() {
  int vertical = (WIDTH - config.back_deadzone.left - config.back_deadzone.right) / 2 + config.back_deadzone.left,
      horizontal = (HEIGHT - config.back_deadzone.top - config.back_deadzone.bottom) / 2 + config.back_deadzone.top;

  vita2d_draw_rectangle(
      config.back_deadzone.left,
      config.back_deadzone.top,
      WIDTH - config.back_deadzone.right - config.back_deadzone.left,
      HEIGHT - config.back_deadzone.bottom - config.back_deadzone.top,
      0x3000ff00
      );

  vita2d_draw_line(vertical, config.back_deadzone.top, vertical, HEIGHT - config.back_deadzone.bottom, 0xffffffff);
  vita2d_draw_line(config.back_deadzone.left, horizontal, WIDTH - config.back_deadzone.right, horizontal, 0xffffffff);

  SceTouchData touch_data;
  sceTouchPeek(SCE_TOUCH_PORT_BACK, &touch_data, 1);

  for (int i = 0; i < touch_data.reportNum; i++) {
    int x = lerp(touch_data.report[i].x, 1919, 960);
    int y = lerp(touch_data.report[i].y, 1087, 544);
    if (x < config.back_deadzone.left || x > WIDTH - config.back_deadzone.right)
      continue;

    if (y < config.back_deadzone.top || y > HEIGHT - config.back_deadzone.bottom)
      continue;

    vita2d_draw_fill_circle(x, y, 30, 0xffffffff);
  }
}

static int deadzone_loop(int cursor, void *context, const input_data *input) {
  menu_entry *menu = context;

  bool left = input->buttons & SCE_CTRL_LEFT;
  bool right = input->buttons & SCE_CTRL_RIGHT;

  int delta = left ? -15 : (right ? 15 : 0);
  switch (cursor) {
    case 0: config.back_deadzone.top += delta; break;
    case 1: config.back_deadzone.right += delta; break;
    case 2: config.back_deadzone.bottom += delta; break;
    case 3: config.back_deadzone.left += delta; break;
  }

  settings_loop_setup = 0;
  char current[256];

  int numbers[] = {
    config.back_deadzone.top,
    config.back_deadzone.right,
    config.back_deadzone.bottom,
    config.back_deadzone.left
  };
  for (int i = 0; i < 4; i++) {
    sprintf(current, "%dpx", numbers[i]);
    strcpy(menu[i].subname, current);
  }

  return 0;
}

static int deadzone_settings_menu() {
  sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, SCE_TOUCH_SAMPLING_STATE_START);

  menu_entry menu[16];
  int idx = 0;
  menu[idx++] = (menu_entry) { .name = "Top: ", .disabled = false, .id = 0, .suffix = "←→" };
  menu[idx++] = (menu_entry) { .name = "Left: ", .disabled = false, .id = 1, .suffix = "←→" };
  menu[idx++] = (menu_entry) { .name = "Bottom: ", .disabled = false, .id = 2, .suffix = "←→" };
  menu[idx++] = (menu_entry) { .name = "Right: ", .disabled = false, .id = 3, .suffix = "←→" };

  menu_geom geom = make_geom_centered(250, 120);
  geom.x = 50;
  geom.y = 50;
  geom.el = 25;
  return display_menu(menu, idx, &geom, &deadzone_loop, NULL, &deadzone_draw, &menu);
}

/*
 * Special keys
 */

static int special_keys_ord(char *text) {
  bool did_find = false;
  int i = 0;
  for (; i < sizeof(settings_special_codes) / sizeof(int); i++) {
    if (strcmp(settings_special_names[i], text) == 0) {
      did_find = true;
      break;
    }
  }

  if (did_find) {
    return settings_special_codes[i];
  } else {
    return 0;
  }
}

static void special_keys_name(int ord, char *text) {
  if (ord == 0) {
    strcpy(text, "None");
    return;
  }

  bool did_find = false;
  int i = 0;
  for (; i < sizeof(settings_special_codes) / sizeof(int); i++) {
    if (settings_special_codes[i] == ord) {
      did_find = true;
      break;
    }
  }

  if (did_find) {
    strcpy(text, settings_special_names[i]);
  } else {
    sprintf(text, "%d", ord);
  }
}

enum {
  SETTINGS_SELECT_SPECIAL_KEY_MANUAL = -1000
};

enum {
  SETTINGS_SPECIAL_KEYS_OFFSET,
  SETTINGS_SPECIAL_KEYS_SIZE
};

enum {
  SETTINGS_SPECIAL_KEYS_NW_VIEW = 3
};

static int select_special_key_loop(int id, void *context, const input_data *input) {
  int *code = context;

  if ((input->buttons & SCE_CTRL_CROSS) == 0 || input->buttons & SCE_CTRL_HOLD) {
    return 0;
  }
  if (id != SETTINGS_SELECT_SPECIAL_KEY_MANUAL) {
    *code = id;
    return 1;
  }
  char key_code_value[512];
  if (ime_dialog(key_code_value, "Enter key code:", "") == 0) {
      int key_code = atoi(key_code_value);
      if (key_code) {
        *code = key_code;
        return 1;
      } else {
        display_error("Incorrect key code entered: %s", key_code_value);
      }
  }

  return 0;
}

static int select_special_key_menu(int *code) {
  // TODO: sizeof(codes) / sizeof(int) ?
  menu_entry menu[64];
  int idx = 0;
  for (int i = 0; i < sizeof(settings_special_codes) / sizeof(int); i++) {
    unsigned int id = settings_special_codes[i];
    menu[idx++] = (menu_entry) {
      .id = id,
      .name = malloc(sizeof(char) * 256),
      .disabled = (id >= INPUT_TYPE_DEF_NAME)
    };
    special_keys_name(id, menu[idx-1].name);
  }

  menu[idx++] = (menu_entry) { .name = "", .disabled = true, .separator = true };
  menu[idx++] = (menu_entry) { .name = "Enter manually ...", .id = SETTINGS_SELECT_SPECIAL_KEY_MANUAL };

  int return_code = display_menu(menu, idx, NULL, &select_special_key_loop, NULL, NULL, code);
  for (int i = 0; i < sizeof(settings_special_codes) / sizeof(int); i++) {
    free(menu[i].name);
  }

  return return_code;
}

static int special_keys_loop(int id, void *context, const input_data *input) {
  bool left = input->buttons & SCE_CTRL_LEFT;
  bool right = input->buttons & SCE_CTRL_RIGHT;
  int selected_ord = -1;
  int delta = left ? -15 : (right ? 15 : 0);

  switch (id) {
    case SETTINGS_SPECIAL_KEYS_OFFSET:
      config.special_keys.offset += delta;
      break;
    case SETTINGS_SPECIAL_KEYS_SIZE:
      config.special_keys.size += delta;
      break;
    default:
      if ((input->buttons & SCE_CTRL_CROSS) == 0 || input->buttons & SCE_CTRL_HOLD) {
        break;
      }
      select_special_key_menu(&selected_ord);
      if (selected_ord != -1) {
        switch (id) {
          case TOUCHSEC_SPECIAL_NW:
            config.special_keys.nw = selected_ord;
            break;
          case TOUCHSEC_SPECIAL_NE:
            config.special_keys.ne = selected_ord;
            break;
          case TOUCHSEC_SPECIAL_SW:
            config.special_keys.sw = selected_ord;
            break;
          case TOUCHSEC_SPECIAL_SE:
            config.special_keys.se = selected_ord;
            break;
        }
      }
      break;
  }

  menu_entry *menu = context;

  int idx = 0;
  sprintf(menu[idx++].subname, "%d", config.special_keys.offset);
  sprintf(menu[idx++].subname, "%d", config.special_keys.size);

  idx++;
  special_keys_name(config.special_keys.nw, menu[idx++].subname);
  special_keys_name(config.special_keys.ne, menu[idx++].subname);
  special_keys_name(config.special_keys.sw, menu[idx++].subname);
  special_keys_name(config.special_keys.se, menu[idx++].subname);

  return 0;
}

static void special_keys_draw() {
  int special_offset = config.special_keys.offset,
      special_size = config.special_keys.size;

  unsigned int color = 0xff006000;

  for (int i = TOUCHSEC_SPECIAL_NW; i <= TOUCHSEC_SPECIAL_SE; i++) {
    switch (i) {
      case TOUCHSEC_SPECIAL_SW:
        vita2d_draw_rectangle(
            special_offset,
            HEIGHT - special_size - special_offset,
            special_size,
            special_size,
            color);
      case TOUCHSEC_SPECIAL_SE:
        vita2d_draw_rectangle(
            WIDTH - special_size - special_offset,
            HEIGHT - special_size - special_offset,
            special_size,
            special_size,
            color);
      case TOUCHSEC_SPECIAL_NW:
        vita2d_draw_rectangle(
            special_offset,
            special_offset,
            special_size,
            special_size,
            color);
      case TOUCHSEC_SPECIAL_NE:
        vita2d_draw_rectangle(
            WIDTH - special_size - special_offset,
            special_offset,
            special_size,
            special_size,
            color);
    }
  }

}

static int special_keys_menu() {
  menu_entry menu[16];
  int idx = 0;

  menu[idx++] = (menu_entry) { .name = "Offset", .id = SETTINGS_SPECIAL_KEYS_OFFSET };
  menu[idx++] = (menu_entry) { .name = "Size", .id = SETTINGS_SPECIAL_KEYS_SIZE };
  menu[idx++] = (menu_entry) { .name = "Assignments", .disabled = true, .separator = false };
  menu[idx++] = (menu_entry) { .name = "Top left", .id = TOUCHSEC_SPECIAL_NW };
  menu[idx++] = (menu_entry) { .name = "Top right", .id = TOUCHSEC_SPECIAL_NE };
  menu[idx++] = (menu_entry) { .name = "Bottom left", .id = TOUCHSEC_SPECIAL_SW };
  menu[idx++] = (menu_entry) { .name = "Bottom right", .id = TOUCHSEC_SPECIAL_SE };

  return display_menu(menu, idx, NULL, &special_keys_loop, NULL, &special_keys_draw, &menu);
}

/*
 * Main menu
 */

enum {
  SETTINGS_RESOLUTION = 100,
  SETTINGS_FPS,
  SETTINGS_BITRATE,
  SETTINGS_SOPS,
  SETTINGS_ENABLE_FRAME_INVAL,
  SETTINGS_ENABLE_STREAM_OPTIMIZE,
  SETTINGS_SAVE_DEBUG_LOG,
  SETTINGS_DISABLE_POWERSAVE,
  SETTINGS_DISABLE_VSYNC,
  SETTINGS_ENABLE_MAPPING,
  SETTINGS_BACK_DEADZONE,
  SETTINGS_SPECIAL_KEYS,
  SETTINGS_MOUSE_ACCEL,
};

enum {
  SETTINGS_VIEW_RESOLUTION,
  SETTINGS_VIEW_FPS,
  SETTINGS_VIEW_BITRATE,
  SETTINGS_VIEW_SOPS,
  SETTINGS_VIEW_ENABLE_FRAME_INVAL,
  SETTINGS_VIEW_ENABLE_STREAM_OPTIMIZE,
  SETTINGS_VIEW_SAVE_DEBUG_LOG,
  SETTINGS_VIEW_DISABLE_POWERSAVE,
  SETTINGS_VIEW_DISABLE_VSYNC,
  SETTINGS_VIEW_ENABLE_MAPPING,
  SETTINGS_VIEW_BACK_DEADZONE,
  SETTINGS_VIEW_SPECIAL_KEYS,
  SETTINGS_VIEW_MOUSE_ACCEL,
};

static int SETTINGS_VIEW_IDX[10];

static int move_idx_in_array(char *array[], int count, char *find, int index_dist) {
  int i = 0;
  for (; i < count; i++) {
    if (strcmp(find, array[i]) == 0) {
      i += index_dist;
      break;
    }
  }

  if (i >= count) {
    return count - 1;
  } else if (i < 0) {
    return 0;
  } else {
    return i;
  }
}

static int settings_loop(int id, void *context, const input_data *input) {
  menu_entry *menu = context;
  bool did_change = 0;
  bool left = (input->buttons & SCE_CTRL_LEFT) && (input->buttons & SCE_CTRL_HOLD) == 0;
  bool right = (input->buttons & SCE_CTRL_RIGHT) && (input->buttons & SCE_CTRL_HOLD) == 0;

  char current[256];
  int new_idx;

  switch (id) {
    case SETTINGS_RESOLUTION:
      if (!left && !right) {
          break;
      }
      char *resolutions[] = {"960x544", "1280x720", "1920x1080"};
      sprintf(current, "%dx%d", config.stream.width, config.stream.height);

      new_idx = move_idx_in_array(resolutions, 3, current, left ? -1 : +1);

      switch (new_idx) {
        case 0: config.stream.width = 960; config.stream.height = 544; break;
        case 1: config.stream.width = 1280; config.stream.height = 720; break;
        case 2: config.stream.width = 1920; config.stream.height = 1080; break;
      }

      did_change = 1;
      break;
    case SETTINGS_FPS:
      if (!left && !right) {
          break;
      }
      char *settings[] = {"30", "60"};
      sprintf(current, "%d", config.stream.fps);
      new_idx = move_idx_in_array(settings, 2, current, left ? -1 : +1);

      switch (new_idx) {
        case 0: config.stream.fps = 30; break;
        case 1: config.stream.fps = 60; break;
      }

      did_change = 1;
      break;
    case SETTINGS_BITRATE:
      if ((input->buttons & SCE_CTRL_CROSS) == 0 || input->buttons & SCE_CTRL_HOLD) {
        break;
      }
      char value[512];
      int ret;
      if ((ret = ime_dialog(value, "Enter bitrate: ", "")) == 0) {
        int bitrate = atoi(value);
        if (bitrate) {
          config.stream.bitrate = bitrate;
          did_change = 1;
        } else {
          display_error("Incorrect bitrate entered: %s", value);
        }
      }
      break;
    case SETTINGS_SOPS:
      if ((input->buttons & SCE_CTRL_CROSS) == 0 || input->buttons & SCE_CTRL_HOLD) {
        break;
      }
      did_change = 1;
      config.sops = !config.sops;
      break;
    case SETTINGS_ENABLE_FRAME_INVAL:
      if ((input->buttons & SCE_CTRL_CROSS) == 0 || input->buttons & SCE_CTRL_HOLD) {
        break;
      }
      did_change = 1;
      config.enable_ref_frame_invalidation = !config.enable_ref_frame_invalidation;
      break;
    case SETTINGS_ENABLE_STREAM_OPTIMIZE:
      if ((input->buttons & SCE_CTRL_CROSS) == 0 || input->buttons & SCE_CTRL_HOLD) {
        break;
      }
      did_change = 1;
      config.stream.streamingRemotely = config.stream.streamingRemotely ? 0 : 1;
      break;
    case SETTINGS_SAVE_DEBUG_LOG:
      if ((input->buttons & SCE_CTRL_CROSS) == 0 || input->buttons & SCE_CTRL_HOLD) {
        break;
      }
      did_change = 1;
      config.save_debug_log = !config.save_debug_log;
      break;
    case SETTINGS_DISABLE_POWERSAVE:
      if ((input->buttons & SCE_CTRL_CROSS) == 0 || input->buttons & SCE_CTRL_HOLD) {
        break;
      }
      did_change = 1;
      config.disable_powersave = !config.disable_powersave;
      break;
    case SETTINGS_DISABLE_VSYNC:
      if ((input->buttons & SCE_CTRL_CROSS) == 0 || input->buttons & SCE_CTRL_HOLD) {
        break;
      }
      did_change = 1;
      config.disable_vsync = !config.disable_vsync;
      break;
    case SETTINGS_ENABLE_MAPPING:
      if ((input->buttons & SCE_CTRL_CROSS) == 0 || input->buttons & SCE_CTRL_HOLD) {
        break;
      }
      did_change = 1;
      if (config.mapping) {
        free(config.mapping);
        config.mapping = NULL;
      } else {
        config.mapping = strdup("mappings/vita.conf");
      }
      break;
    case SETTINGS_BACK_DEADZONE:
      if ((input->buttons & SCE_CTRL_CROSS) == 0 || input->buttons & SCE_CTRL_HOLD) {
        break;
      }
      deadzone_settings_menu();
      did_change = 1;
      break;
    case SETTINGS_SPECIAL_KEYS:
      if ((input->buttons & SCE_CTRL_CROSS) == 0 || input->buttons & SCE_CTRL_HOLD) {
        break;
      }
      special_keys_menu();
      break;
    case SETTINGS_MOUSE_ACCEL:
      left = input->buttons & SCE_CTRL_LEFT;
      right = input->buttons & SCE_CTRL_RIGHT;
      if (!left && !right) {
          break;
      }
      if (left) {
        config.mouse_acceleration -= 15;
        if (config.mouse_acceleration < 0) {
          config.mouse_acceleration = 0;
        }
      } else {
        config.mouse_acceleration += 15;
        if (config.mouse_acceleration > 300) {
          config.mouse_acceleration = 300;
        }
      }

      did_change = 1;
      break;

  }

  if (!did_change && !settings_loop_setup) {
    return 0;
  }
  settings_loop_setup = 0;

#define MENU_REPLACE(ID, MESSAGE) \
    strcpy(menu[SETTINGS_VIEW_IDX[(ID)]].subname, (MESSAGE))

  sprintf(current, "%dx%d", config.stream.width, config.stream.height);
  MENU_REPLACE(SETTINGS_VIEW_RESOLUTION, current);

  sprintf(current, "%d", config.stream.fps);
  MENU_REPLACE(SETTINGS_VIEW_FPS, current);

  sprintf(current, "%d", config.stream.bitrate);
  MENU_REPLACE(SETTINGS_VIEW_BITRATE, current);

  sprintf(current, "%s", config.sops ? "yes" : "no");
  MENU_REPLACE(SETTINGS_VIEW_SOPS, current);

  sprintf(current, "%s", config.enable_ref_frame_invalidation ? "yes" : "no");
  MENU_REPLACE(SETTINGS_VIEW_ENABLE_FRAME_INVAL, current);

  sprintf(current, "%s", config.stream.streamingRemotely ? "yes" : "no");
  MENU_REPLACE(SETTINGS_VIEW_ENABLE_STREAM_OPTIMIZE, current);

  sprintf(current, "%s", config.disable_powersave ? "yes" : "no");
  MENU_REPLACE(SETTINGS_VIEW_DISABLE_POWERSAVE, current);

  sprintf(current, "%s", config.disable_vsync ? "yes" : "no");
  MENU_REPLACE(SETTINGS_VIEW_DISABLE_VSYNC, current);

  sprintf(current, "%s", config.save_debug_log ? "yes" : "no");
  MENU_REPLACE(SETTINGS_VIEW_SAVE_DEBUG_LOG, current);

  sprintf(current, "%s", config.mapping != 0 ? "yes" : "no");
  MENU_REPLACE(SETTINGS_VIEW_ENABLE_MAPPING, current);

  sprintf(current, "%dpx,%dpx,%dpx,%dpx",
          config.back_deadzone.top,
          config.back_deadzone.right,
          config.back_deadzone.bottom,
          config.back_deadzone.left);
  MENU_REPLACE(SETTINGS_VIEW_BACK_DEADZONE, current);

  sprintf(current, "%d", config.mouse_acceleration);
  MENU_REPLACE(SETTINGS_VIEW_MOUSE_ACCEL, current);
  return 0;
}

static int settings_back(void *context) {
  ui_settings_save_config();
  return 0;
}

int ui_settings_menu() {
  menu_entry menu[32];
  int idx = 0;
#define MENU_CATEGORY(NAME) \
  do { \
    menu[idx] = (menu_entry) { .name = (NAME), .disabled = true, .separator = true }; \
    idx++; \
  } while (0)
#define MENU_ENTRY(ID, TAG, NAME, SUFFIX) \
  do { \
    menu[idx] = (menu_entry) { .name = (NAME), .id = (ID), .suffix = (SUFFIX) }; \
    SETTINGS_VIEW_IDX[(TAG)] = idx; \
    idx++; \
  } while(0)
#define MENU_MESSAGE(MESSAGE) \
  do { \
    menu[idx] = (menu_entry) { .name = "", .disabled = true, .subname = (MESSAGE) }; \
    idx++; \
  } while(0)

#define LEFT_RIGHT_ARROWS "\xe2\x86\x90\xe2\x86\x92"

  MENU_CATEGORY("Stream");
  MENU_ENTRY(SETTINGS_RESOLUTION, SETTINGS_VIEW_RESOLUTION, "Resolution", LEFT_RIGHT_ARROWS);
  MENU_ENTRY(SETTINGS_FPS, SETTINGS_VIEW_FPS, "FPS", LEFT_RIGHT_ARROWS);
  MENU_ENTRY(SETTINGS_BITRATE, SETTINGS_VIEW_BITRATE, "Bitrate", "");
  MENU_ENTRY(SETTINGS_SOPS, SETTINGS_VIEW_SOPS, "Change graphical game settings for performance", "");
  MENU_ENTRY(SETTINGS_ENABLE_FRAME_INVAL, SETTINGS_VIEW_ENABLE_FRAME_INVAL, "Enable reference frame invalidation", "");
  MENU_ENTRY(SETTINGS_ENABLE_STREAM_OPTIMIZE, SETTINGS_VIEW_ENABLE_STREAM_OPTIMIZE, "Enable stream optimization", "");
  MENU_ENTRY(SETTINGS_DISABLE_VSYNC, SETTINGS_VIEW_DISABLE_VSYNC, "Disable V-Sync", "");

  MENU_CATEGORY("System");
  MENU_ENTRY(SETTINGS_SAVE_DEBUG_LOG, SETTINGS_VIEW_SAVE_DEBUG_LOG, "Enable debug log", "");
  MENU_ENTRY(SETTINGS_DISABLE_POWERSAVE, SETTINGS_VIEW_DISABLE_POWERSAVE, "Disable power save", "");

  MENU_CATEGORY("Input");
  MENU_ENTRY(SETTINGS_MOUSE_ACCEL, SETTINGS_VIEW_MOUSE_ACCEL, "Mouse acceleration", LEFT_RIGHT_ARROWS);
  MENU_ENTRY(SETTINGS_ENABLE_MAPPING, SETTINGS_VIEW_ENABLE_MAPPING, "Enable mapping file", "");
  MENU_MESSAGE("Located at ux0:data/moonlight/mappings/vita.conf");
  MENU_MESSAGE("Example in github repo.");
  MENU_ENTRY(SETTINGS_BACK_DEADZONE, SETTINGS_VIEW_BACK_DEADZONE, "Back touchscreen deadzone", "");
  MENU_ENTRY(SETTINGS_SPECIAL_KEYS, SETTINGS_VIEW_SPECIAL_KEYS, "Touchscreen special keys", "");

  settings_loop_setup = 1;
  assert(idx < 32);
  return display_menu(menu, idx, NULL, &settings_loop, &settings_back, NULL, &menu);
}

void ui_settings_save_config() {
  config_save(config_path, &config);
}

