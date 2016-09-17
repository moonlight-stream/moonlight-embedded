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

static unsigned int settings_special_codes[] = {0, INPUT_SPECIAL_KEY_PAUSE | INPUT_SPECIAL_MASK,
  1024 | INPUT_GAMEPAD_MASK ,
  256 | INPUT_GAMEPAD_MASK,
  512 | INPUT_GAMEPAD_MASK,
  64 | INPUT_GAMEPAD_MASK,
  128 | INPUT_GAMEPAD_MASK,
  BUTTON_LEFT | INPUT_MOUSE_MASK,
  BUTTON_RIGHT | INPUT_MOUSE_MASK,
  BUTTON_MIDDLE | INPUT_MOUSE_MASK,
  27,    73,  77,  9,     112,  113,  114,  115,  116,  117,  118,  119, 120,   121,   122,   123 };
static char *settings_special_names[] = {"None", "Pause stream",
  "Special (XBox button)", "LB", "RB", "LS", "RS",
  "LMB", "RMB", "MMB",
  "Esc", "I", "M", "Tab", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12" };
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

static int deadzone_loop(int cursor, void *context) {
  struct menu_entry *menu = context;
  int did_change = 1;

  bool left = was_button_pressed(SCE_CTRL_LEFT), right = was_button_pressed(SCE_CTRL_RIGHT);
  if (left || right) {
    int delta = left ? -15 : (right ? 15 : 0);
    switch (cursor) {
      case 0: config.back_deadzone.top += delta; break;
      case 1: config.back_deadzone.right += delta; break;
      case 2: config.back_deadzone.bottom += delta; break;
      case 3: config.back_deadzone.left += delta; break;
    }
  }

  if (did_change) {
    settings_loop_setup = 0;
    char current[256];

    int numbers[] = {config.back_deadzone.top, config.back_deadzone.right, config.back_deadzone.bottom, config.back_deadzone.left };
    for (int i = 0; i < 4; i++) {
      sprintf(current, "%dpx", numbers[i]);
      strcpy(menu[i].subname, current);
    }
  }

  return 0;
}

static int deadzone_settings_menu() {
  sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, SCE_TOUCH_SAMPLING_STATE_START);

  struct menu_entry menu[16];
  int idx = 0;
  menu[idx++] = (struct menu_entry) { .name = "Top: ", .disabled = false, .id = 0, .suffix = "←→" };
  menu[idx++] = (struct menu_entry) { .name = "Left: ", .disabled = false, .id = 1, .suffix = "←→" };
  menu[idx++] = (struct menu_entry) { .name = "Bottom: ", .disabled = false, .id = 2, .suffix = "←→" };
  menu[idx++] = (struct menu_entry) { .name = "Right: ", .disabled = false, .id = 3, .suffix = "←→" };

  struct menu_geom geom = make_geom_centered(250, 120);
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

static int select_special_key_loop(int id, void *context) {
  int *code = context;

  if (was_button_pressed(SCE_CTRL_CROSS)) {
    if (id == SETTINGS_SELECT_SPECIAL_KEY_MANUAL) {
      char key_code_value[512];
      if (ime_dialog(&key_code_value, "Enter key code:", "") == 0) {
          int key_code = atoi(key_code_value);
          if (key_code) {
            *code = key_code;
            return 1;
          } else {
            display_error("Incorrect key code entered: %s", key_code_value);
          }
      }
    } else {
      *code = id;
      return 1;
    }
  }

  return 0;
}

static int select_special_key_menu(int *code) {
  struct menu_entry menu[64];
  int idx = 0;
  for (int i = 0; i < sizeof(settings_special_codes) / sizeof(int); i++) {
    menu[idx++] = (struct menu_entry) { .name = malloc(sizeof(char) * 256), .id = settings_special_codes[i] };
    special_keys_name(settings_special_codes[i], menu[idx-1].name);
  }

  menu[idx++] = (struct menu_entry) { .name = "", .disabled = true, .separator = true };
  menu[idx++] = (struct menu_entry) { .name = "Enter manually ...", .id = SETTINGS_SELECT_SPECIAL_KEY_MANUAL };

  int return_code = display_menu(menu, idx, NULL, &select_special_key_loop, NULL, NULL, code);
  for (int i = 0; i < sizeof(settings_special_codes) / sizeof(int); i++) {
    free(menu[i].name);
  }

  return return_code;
}

static int special_keys_loop(int id, void *context) {
  bool did_change = true;

  bool left = was_button_pressed(SCE_CTRL_LEFT), right = was_button_pressed(SCE_CTRL_RIGHT);
  int selected_ord = -1;

  switch (id) {
    case SETTINGS_SPECIAL_KEYS_OFFSET:
    case SETTINGS_SPECIAL_KEYS_SIZE:
      if (left || right) {
        int delta = left ? -15 : (right ? 15 : 0);
        switch (id) {
          case SETTINGS_SPECIAL_KEYS_OFFSET: config.special_keys.offset += delta; break;
          case SETTINGS_SPECIAL_KEYS_SIZE: config.special_keys.size += delta; break;
        }
      }

      break;
    default:
      if (was_button_pressed(SCE_CTRL_CROSS)) {
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
      } break;
  }

  if (did_change) {
    struct menu_entry *menu = context;
    int idx = 0;
    sprintf(menu[idx++].subname, "%d", config.special_keys.offset);
    sprintf(menu[idx++].subname, "%d", config.special_keys.size);

    idx++;
    special_keys_name(config.special_keys.nw, menu[idx++].subname);
    special_keys_name(config.special_keys.ne, menu[idx++].subname);
    special_keys_name(config.special_keys.sw, menu[idx++].subname);
    special_keys_name(config.special_keys.se, menu[idx++].subname);
  }

  return 0;
}

static void special_keys_draw() {
  int special_offset = config.special_keys.offset,
      special_size = config.special_keys.size;

  unsigned int color = 0xffffffff;

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
  struct menu_entry menu[16];
  int idx = 0;

  menu[idx++] = (struct menu_entry) { .name = "Offset", .id = SETTINGS_SPECIAL_KEYS_OFFSET };
  menu[idx++] = (struct menu_entry) { .name = "Size", .id = SETTINGS_SPECIAL_KEYS_SIZE };
  menu[idx++] = (struct menu_entry) { .name = "Assignments", .disabled = true, .separator = false };
  menu[idx++] = (struct menu_entry) { .name = "Top left", .id = TOUCHSEC_SPECIAL_NW };
  menu[idx++] = (struct menu_entry) { .name = "Top right", .id = TOUCHSEC_SPECIAL_NE };
  menu[idx++] = (struct menu_entry) { .name = "Bottom left", .id = TOUCHSEC_SPECIAL_SW };
  menu[idx++] = (struct menu_entry) { .name = "Bottom right", .id = TOUCHSEC_SPECIAL_SE };

  return display_menu(menu, idx, NULL, &special_keys_loop, NULL, &special_keys_draw, &menu);
}

/*
 * Main menu
 */

enum {
  SETTINGS_RESOLUTION = 100,
  SETTINGS_FPS,
  SETTINGS_BITRATE,
  SETTINGS_FRONTTOUCHSCREEN,
  SETTINGS_DISABLE_POWERSAVE,
  SETTINGS_ENABLE_MAPPING,
  SETTINGS_BACK_DEADZONE,
  SETTINGS_SPECIAL_KEYS
};

enum {
  SETTINGS_VIEW_RESOLUTION = 1,
  SETTINGS_VIEW_FPS,
  SETTINGS_VIEW_BITRATE,
  SETTINGS_VIEW_FRONTTOUCHSCREEN = 5,
  SETTINGS_VIEW_DISABLE_POWERSAVE = 7,
  SETTINGS_VIEW_ENABLE_MAPPING,
  SETTINGS_VIEW_BACK_DEADZONE = 11
};

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

static int settings_loop(int id, void *context) {
  struct menu_entry *menu = context;
  bool did_change = 0;
  bool left, right;
  switch (id) {
    case SETTINGS_RESOLUTION:
      left = was_button_pressed(SCE_CTRL_LEFT), right = was_button_pressed(SCE_CTRL_RIGHT);
      if (left || right) {
        char *resolutions[] = {"960x544", "1280x720", "1920x1080"};
        char current[256];
        sprintf(current, "%dx%d", config.stream.width, config.stream.height);

        int new_idx = move_idx_in_array(
            resolutions,
            3,
            current,
            left ? -1 : +1
            );

        switch (new_idx) {
          case 0: config.stream.width = 960; config.stream.height = 544; break;
          case 1: config.stream.width = 1280; config.stream.height = 720; break;
          case 2: config.stream.width = 1920; config.stream.height = 1080; break;
        }

        did_change = 1;
      } break;
    case SETTINGS_FPS:
      left = was_button_pressed(SCE_CTRL_LEFT), right = was_button_pressed(SCE_CTRL_RIGHT);
      if (left || right) {
        char *settings[] = {"30", "60"};
        char current[256];
        sprintf(current, "%d", config.stream.fps);
        int new_idx = move_idx_in_array(
            settings,
            2,
            current,
            left ? -1 : +1
            );

        switch (new_idx) {
          case 0: config.stream.fps = 30; break;
          case 1: config.stream.fps = 60; break;
        }

        did_change = 1;
      } break;
    case SETTINGS_BITRATE:
      if (was_button_pressed(SCE_CTRL_CROSS)) {
        char value[512];
        int ret;
        if ((ret = ime_dialog(&value, "Enter bitrate: ", "")) == 0) {
          int bitrate = atoi(value);
          if (bitrate) {
            config.stream.bitrate = bitrate;
            did_change = 1;
          } else {
            display_error("Incorrect bitrate entered: %s", value);
          }
        }
      } break;
    case SETTINGS_FRONTTOUCHSCREEN:
      if (was_button_pressed(SCE_CTRL_CROSS)) {
        did_change = 1;
        config.fronttouchscreen_buttons = !config.fronttouchscreen_buttons;
      } break;
    case SETTINGS_DISABLE_POWERSAVE:
      if (was_button_pressed(SCE_CTRL_CROSS)) {
        did_change = 1;
        config.disable_powersave = !config.disable_powersave;
      } break;
    case SETTINGS_ENABLE_MAPPING:
      if (was_button_pressed(SCE_CTRL_CROSS)) {
        did_change = 1;
        if (config.mapping) {
          config.mapping = 0;
        } else {
          config.mapping = "mappings/vita.conf";
        }
      } break;
    case SETTINGS_BACK_DEADZONE:
      if (was_button_pressed(SCE_CTRL_CROSS)) {
        deadzone_settings_menu();
        did_change = 1;
      } break;
    case SETTINGS_SPECIAL_KEYS:
      if (was_button_pressed(SCE_CTRL_CROSS)) {
        special_keys_menu();
      } break;
  }

  if (did_change || settings_loop_setup) {
    settings_loop_setup = 0;
    char current[256];

    sprintf(current, "%dx%d", config.stream.width, config.stream.height);
    strcpy(menu[SETTINGS_VIEW_RESOLUTION].subname, current);

    sprintf(current, "%d", config.stream.fps);
    strcpy(menu[SETTINGS_VIEW_FPS].subname, current);

    sprintf(current, "%d", config.stream.bitrate);
    strcpy(menu[SETTINGS_VIEW_BITRATE].subname, current);

    sprintf(current, "%s", config.fronttouchscreen_buttons ? "yes" : "no");
    strcpy(menu[SETTINGS_VIEW_FRONTTOUCHSCREEN].subname, current);

    sprintf(current, "%s", config.disable_powersave ? "yes" : "no");
    strcpy(menu[SETTINGS_VIEW_DISABLE_POWERSAVE].subname, current);

    sprintf(current, "%s", config.mapping != 0 ? "yes" : "no");
    strcpy(menu[SETTINGS_VIEW_ENABLE_MAPPING].subname, current);

    sprintf(
        current,
        "%dpx,%dpx,%dpx,%dpx",
        config.back_deadzone.top,
        config.back_deadzone.right,
        config.back_deadzone.bottom,
        config.back_deadzone.left);
    strcpy(menu[SETTINGS_VIEW_BACK_DEADZONE].subname, current);
  }

  return 0;
}

static int settings_back(void *context) {
  ui_settings_save_config();
  return 0;
}

int ui_settings_menu() {
  struct menu_entry menu[32];
  int idx = 0;
  menu[idx++] = (struct menu_entry) { .name = "Stream", .disabled = true, .separator = true };
  idx++; menu[SETTINGS_VIEW_RESOLUTION] = (struct menu_entry) { .name = "Resolution", .id = SETTINGS_RESOLUTION, .suffix = "←→"};
  idx++; menu[SETTINGS_VIEW_FPS] = (struct menu_entry) { .name = "FPS", .id = SETTINGS_FPS, .suffix = "←→" };
  idx++; menu[SETTINGS_VIEW_BITRATE] = (struct menu_entry) { .name = "Bitrate", .id = SETTINGS_BITRATE };

  // ---------
  menu[idx++] = (struct menu_entry) { .name = "Input", .disabled = true, .separator = true };
  idx++; menu[SETTINGS_VIEW_FRONTTOUCHSCREEN] = (struct menu_entry) { .name = "Use front touchscreen for buttons", .id = SETTINGS_FRONTTOUCHSCREEN };
  menu[idx++] = (struct menu_entry) { .name = "", .disabled = true, .subname = "Disables mouse input and special keys." };
  idx++; menu[SETTINGS_VIEW_DISABLE_POWERSAVE] = (struct menu_entry) { .name = "Disable power save", .id = SETTINGS_DISABLE_POWERSAVE };
  idx++; menu[SETTINGS_VIEW_ENABLE_MAPPING] = (struct menu_entry) { .name = "Enable mapping file", .id = SETTINGS_ENABLE_MAPPING };

  menu[idx++] = (struct menu_entry) { .name = "", .disabled = true, .subname = "Located at ux0:data/moonlight/mappings/vita.conf" };
  menu[idx++] = (struct menu_entry) { .name = "", .disabled = true, .subname = "Example in github repo." };
  idx++; menu[SETTINGS_VIEW_BACK_DEADZONE] = (struct menu_entry) { .name = "Back touchscreen deadzone", .id = SETTINGS_BACK_DEADZONE };
  menu[idx++] = (struct menu_entry) { .name = "Touchscreen special keys", .id = SETTINGS_SPECIAL_KEYS };

  settings_loop_setup = 1;
  assert(idx < 32);
  return display_menu(menu, idx, NULL, &settings_loop, &settings_back, NULL, &menu);
}

void ui_settings_save_config() {
  config_save(config_path, &config);
}

