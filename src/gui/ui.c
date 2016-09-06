#include "ui.h"
#include "guilib.h"
#include "ime.h"

#include "../loop.h"
#include "../connection.h"
#include "../configuration.h"
#include "../audio.h"
#include "../video.h"
#include "../config.h"
#include "../sdl.h"

#include "client.h"
#include "discover.h"
#include "../platform.h"

#include "../input/vita.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>

#include <psp2/net/net.h>
#include <psp2/ctrl.h>
#include <psp2/rtc.h>
#include <psp2/touch.h>
#include <vita2d.h>

static SERVER_DATA server;
static PAPP_LIST server_applist;
static bool server_connected;

/*
 * Main menu
 */

enum {
  MAIN_MENU_CONNECT = 100,
  MAIN_MENU_CONNECT_SAVED,
  MAIN_MENU_SETTINGS,
  MAIN_MENU_QUIT
};

int main_menu_loop(int cursor, void *context) {
  if (was_button_pressed(SCE_CTRL_CROSS)) {
    switch (cursor) {
      case MAIN_MENU_CONNECT:
        __connect_ip();
        return 2;
        break;
      case MAIN_MENU_CONNECT_SAVED:
        __connect_saved();
        return 2;
        break;
      case MAIN_MENU_SETTINGS:
        __settings();
        break;
      case MAIN_MENU_QUIT:
        exit(0);
        break;
    }
  }

  return 0;
}

int main_menu_back(void *context) {
  return 1;
}

int __main_menu() {
  struct menu_entry menu[16];
  int idx = 0;

  menu[idx++] = (struct menu_entry) { .name = "", .subname = "Moonlight Alpha", .disabled = true, .color = 0xff00aa00 };
  if (server_connected) {
    char name[256];
    sprintf(name, "Resume connection to %s", server.address);
    menu[idx++] = (struct menu_entry) { .name = name, .id = MAIN_MENU_CONNECT_SAVED };
  } else if (config.address) {
    char name[256];
    sprintf(name, "Connect to %s", config.address ? config.address : "none");
    menu[idx++] = (struct menu_entry) { .name = name, .id = MAIN_MENU_CONNECT_SAVED };
  }
  menu[idx++] = (struct menu_entry) { .name = "Connect to ...", .id = MAIN_MENU_CONNECT };
  menu[idx++] = (struct menu_entry) { .name = "", .disabled = true, .separator = true };
  menu[idx++] = (struct menu_entry) { .name = "Settings", .id = MAIN_MENU_SETTINGS };
  menu[idx++] = (struct menu_entry) { .name = "Quit", .id = MAIN_MENU_QUIT };

  return display_menu(menu, idx, NULL, &main_menu_loop, &main_menu_back, NULL, NULL);
}

/*
 * Settings
 */

int move_idx_in_array(char *array[], int count, char *find, int index_dist) {
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

void settings_save_config() {
  config_save(config_path, &config);
}

enum {
  SETTINGS_RESOLUTION = 100,
  SETTINGS_FPS,
  SETTINGS_BITRATE,
  SETTINGS_FRONTTOUCHSCREEN,
  SETTINGS_DISABLE_POWERSAVE,
  SETTINGS_ENABLE_MAPPING,
  SETTINGS_BACK_DEADZONE
};

enum {
  SETTINGS_VIEW_RESOLUTION = 1,
  SETTINGS_VIEW_FPS,
  SETTINGS_VIEW_BITRATE,
  SETTINGS_VIEW_FRONTTOUCHSCREEN = 5,
  SETTINGS_VIEW_DISABLE_POWERSAVE,
  SETTINGS_VIEW_ENABLE_MAPPING,
  SETTINGS_VIEW_BACK_DEADZONE = 10
};

static bool settings_loop_setup = 1;
int settings_loop(int id, void *context) {
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
        __deadzone_settings();
        did_change = 1;
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

int settings_back(void *context) {
  settings_save_config();
  return 0;
}

int __settings() {
  struct menu_entry menu[16];
  int idx = 0;
  menu[idx++] = (struct menu_entry) { .name = "Stream", .disabled = true, .separator = true};
  idx++; menu[SETTINGS_VIEW_RESOLUTION] = (struct menu_entry) { .name = "Resolution", .id = SETTINGS_RESOLUTION, .suffix = "←→"};
  idx++; menu[SETTINGS_VIEW_FPS] = (struct menu_entry) { .name = "FPS", .id = SETTINGS_FPS, .suffix = "←→" };
  idx++; menu[SETTINGS_VIEW_BITRATE] = (struct menu_entry) { .name = "Bitrate", .id = SETTINGS_BITRATE };

  // ---------
  menu[idx++] = (struct menu_entry) { .name = "Input", .disabled = true, .separator = true };
  idx++; menu[SETTINGS_VIEW_FRONTTOUCHSCREEN] = (struct menu_entry) { .name = "Use front touchscreen for buttons", .id = SETTINGS_FRONTTOUCHSCREEN };
  idx++; menu[SETTINGS_VIEW_DISABLE_POWERSAVE] = (struct menu_entry) { .name = "Disable power save", .id = SETTINGS_DISABLE_POWERSAVE };
  idx++; menu[SETTINGS_VIEW_ENABLE_MAPPING] = (struct menu_entry) { .name = "Enable mapping file", .id = SETTINGS_ENABLE_MAPPING };

  menu[idx++] = (struct menu_entry) { .name = "", .disabled = true, .subname = "Located at ux0:data/moonlight/mappings/vita.conf" };
  menu[idx++] = (struct menu_entry) { .name = "", .disabled = true, .subname = "Example in github repo." };
  idx++; menu[SETTINGS_VIEW_BACK_DEADZONE] = (struct menu_entry) { .name = "Back touchscreen deadzone", .id = SETTINGS_BACK_DEADZONE };

  settings_loop_setup = 1;
  assert(idx < 16);
  return display_menu(menu, idx, NULL, &settings_loop, &settings_back, NULL, &menu);
}

#define lerp(value, from_max, to_max) ((((value*10) * (to_max*10))/(from_max*10))/10)
void deadzone_draw() {
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

int deadzone_loop(int cursor, void *context) {
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

int __deadzone_settings() {
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
 * Connect
 */

enum {
  CONNECT_PAIRUNPAIR = 13,
  CONNECT_DISCONNECT,
  CONNECT_QUITAPP
};

#define QUIT_RELOAD 2

int connect_loop(int id, void *context) {
  if (was_button_pressed(SCE_CTRL_CROSS)) {
    switch (id) {
      case CONNECT_PAIRUNPAIR:
        if (server.paired) {
          flash_message("Unpairing...");
          int ret = gs_unpair(&server);
          if (ret == GS_OK)
            return QUIT_RELOAD;
          else
            display_error("Unpairing failed: %d", ret);
        } else {
          char pin[5];
          char message[256];
          sprintf(pin, "%d%d%d%d", (int)rand() % 10, (int)rand() % 10, (int)rand() % 10, (int)rand() % 10);
          flash_message("Please enter the following PIN\non the target PC:\n\n%s", pin);

          int ret = gs_pair(&server, &pin[0]);
          if (ret == 0) {
            server_connected = false;
            return QUIT_RELOAD;
          } else {
            display_error("Pairing failed: %d", ret);
          }
        } break;

      case CONNECT_DISCONNECT:
        server_connected = false;
        connection_terminate();
        return 1;

      case CONNECT_QUITAPP:
        flash_message("Quitting...");
        int ret = gs_quit_app(&server);
        if (ret == GS_OK) {
          server.currentGame = 0;
          return QUIT_RELOAD;
        } else {
          display_error("Quitting failed: %d", ret);
        } break;

      default:
        vitapower_config(config);
        vitainput_config(config);

        if (connection_get_status() == LI_READY ||
            connection_get_status() == LI_MINIMIZED && server.currentGame != id) {
          flash_message("Stream starting...");
          server.currentGame = id;
          stream(&server, id);
        } else if (connection_get_status() == LI_MINIMIZED) {
          connection_resume();
        }

        while (connection_get_status() == LI_CONNECTED) {
          sceKernelDelayThread(500 * 1000);
        }

        return QUIT_RELOAD; break;
    }
  }

  return 0;
}

int __connect(char *address) {
  if (!server_connected) {
    server.address = malloc(sizeof(char)*256);
    strcpy(server.address, address);

    flash_message("Connecting to:\n %s...", server.address);
    int ret = gs_init(&server, config.key_dir);
    if (ret == GS_OUT_OF_MEMORY) {
      display_error("Not enough memory");
      return;
    } else if (ret == GS_INVALID) {
      display_error("Invalid data received from server: %s\n", config.address, gs_error);
      return;
    } else if (ret == GS_UNSUPPORTED_VERSION) {
      if (!config.unsupported_version) {
        display_error("Unsupported version: %s\n", gs_error);
        return;
      }
    } else if (ret != GS_OK) {
      display_error("Can't connect to server\n%s", config.address);
      return;
    }

    if (gs_applist(&server, &server_applist) != GS_OK) {
      display_error("Can't get applist!");
      return;
    }


    server_connected = true;
  }

  struct menu_entry menu[32];

  int idx = 0;

  //header
  menu[idx++] = (struct menu_entry) { .name = "Connected to the server:", .disabled = 1, .color = 0xffffffff };
  char server_info[256];
  sprintf(server_info, "IP: %s, GPU %s, API v%d", address, server.gpuType, server.serverMajorVersion);
  menu[idx++] = (struct menu_entry) { .name = server_info, .disabled = 1, .color = 0xffffffff };
  menu[idx++] = (struct menu_entry) { .name = "", .disabled = 1 };

  // current stream
  if (server.currentGame != 0) {
    char current_appname[256];
    char current_status[256];

    if (!get_app_name(server_applist, server.currentGame, &current_appname)) {
      strcpy(current_appname, "unknown");
    }
    sprintf(current_status, "Streaming %s", current_appname);

    menu[idx++] = (struct menu_entry) { .name = current_status, .disabled = true, .separator = true };
    menu[idx++] = (struct menu_entry) { .name = "Resume", .id = server.currentGame };
    menu[idx++] = (struct menu_entry) { .name = "Quit", .id = CONNECT_QUITAPP };
  }

  // pairing
  menu[idx++] = (struct menu_entry) { .name = server.paired ? "Paired" : "Not paired", .disabled = true, .separator = true };
  if (server.paired) {
    menu[idx++] = (struct menu_entry) { .name = "Unpair", .id = CONNECT_PAIRUNPAIR };
  } else {
    menu[idx++] = (struct menu_entry) { .name = "Pair", .id = CONNECT_PAIRUNPAIR };
  }
  menu[idx++] = (struct menu_entry) { .name = "Disconnect", .id = CONNECT_DISCONNECT };

  // app list
  if (server_applist != NULL) {
    menu[idx++] = (struct menu_entry) { .name = "Applications", .disabled = true, .separator = true };

    PAPP_LIST list = server_applist;
    while (list) {
      menu[idx++] = (struct menu_entry) { .name = list->name, .id = list->id };
      list = list->next;
    }
  }

  assert(idx < 32);
  return display_menu(menu, idx, NULL, &connect_loop, NULL, NULL, NULL);
}

void __connect_saved() {
  while (__connect(config.address) == 2);
}

void __connect_ip() {
  char ip[512];
  switch (ime_dialog(&ip, "Enter IP:", "192.168.")) {
    case 0:
      server_connected = false;
      config.address = malloc(sizeof(char) * strlen(ip));
      strcpy(config.address, ip);
      settings_save_config();
      __connect_saved();
      break;
    default:
      return;
  }
}
/*
 * Actions
 */

int get_app_id(PAPP_LIST list, char *name) {
  while (list != NULL) {
    if (strcmp(list->name, name) == 0)
      return list->id;

    list = list->next;
  }
  return -1;
}

int get_app_name(PAPP_LIST list, int id, char *name) {
  while (list != NULL) {
    if (list->id == id) {
      strcpy(name, list->name);
      return 1;
    }

    list = list->next;
  }

  return 0;
}

void stream(PSERVER_DATA server, int appId) {
  //int ret = sceNetCtlInit();
  int ret = gs_start_app(server, &config.stream, appId, config.sops, config.localaudio);
  if (ret < 0) {
    if (ret == GS_NOT_SUPPORTED_4K)
      display_error("Server doesn't support 4K\n");
    else
      display_error("Errorcode starting app: %d\n", ret);

    return;
  }

  enum platform system = VITA;
  int drFlags = 0;

  if (config.fullscreen)
    drFlags |= DISPLAY_FULLSCREEN;
  if (config.forcehw)
    drFlags |= FORCE_HARDWARE_ACCELERATION;

  ret = LiStartConnection(
      server->address,
      &config.stream,
      &connection_callbacks,
      platform_get_video(system),
      platform_get_audio(system),
      NULL,
      drFlags,
      server->serverMajorVersion
      );

  if (ret == 0) {
  } else {
    char *stage;
    switch (connection_failed_stage) {
      case STAGE_PLATFORM_INIT: stage = "Platform init"; break;
      case STAGE_NAME_RESOLUTION: stage = "Name resolution"; break;
      case STAGE_RTSP_HANDSHAKE: stage = "RTSP handshake"; break;
      case STAGE_CONTROL_STREAM_INIT: stage = "Control stream init"; break;
      case STAGE_VIDEO_STREAM_INIT: stage = "Video stream init"; break;
      case STAGE_AUDIO_STREAM_INIT: stage = "Audio stream init"; break;
      case STAGE_CONTROL_STREAM_START: stage = "Control stream start"; break;
      case STAGE_VIDEO_STREAM_START: stage = "Video stream start"; break;
      case STAGE_AUDIO_STREAM_START: stage = "Audio stream start"; break;
      case STAGE_INPUT_STREAM_START: stage = "Input stream start"; break;
    }

    display_error("Failed to start stream: error code %d\nFailed stage: %s\n(error code %d)", ret, stage, connection_failed_stage_code);
    return;
  }
}

void gui_init() {
  guilib_init();
}

void gui_loop() {
  gui_init();

  while (__main_menu() == 2);

  vita2d_fini();
}
