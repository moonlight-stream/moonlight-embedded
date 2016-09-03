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

#include <psp2/ctrl.h>
#include <vita2d.h>

static SERVER_DATA server;

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
        break;
      case MAIN_MENU_CONNECT_SAVED:
        __connect_saved();
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

  if (config.address) {
    char prev[256];
    sprintf(prev, "Connect to %s", config.address ? config.address : "none");
    menu[idx++] = (struct menu_entry) { .name = prev, .id = MAIN_MENU_CONNECT_SAVED };
  }
  menu[idx++] = (struct menu_entry) { .name = "Connect to ...", .id = MAIN_MENU_CONNECT };
  menu[idx++] = (struct menu_entry) { .name = "", .disabled = true, .separator = true };
  menu[idx++] = (struct menu_entry) { .name = "Settings", .id = MAIN_MENU_SETTINGS };
  menu[idx++] = (struct menu_entry) { .name = "Quit", .id = MAIN_MENU_QUIT };

  return display_menu(menu, idx, &main_menu_loop, &main_menu_back, NULL);
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
  SETTINGS_DISABLE_POWERSAVE
};

enum {
  SETTINGS_VIEW_RESOLUTION = 1,
  SETTINGS_VIEW_FPS,
  SETTINGS_VIEW_BITRATE,
  SETTINGS_VIEW_FRONTTOUCHSCREEN = 5,
  SETTINGS_VIEW_DISABLE_POWERSAVE
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
        if (ime_dialog(&value, "Enter bitrate: ", 0) == 0) {
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
  }

  if (did_change) {
    settings_save_config();
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
  }

  return 0;
}

int __settings() {
  struct menu_entry menu[16];
  menu[0] = (struct menu_entry) { .name = "Stream", .disabled = true, .separator = true};
  menu[SETTINGS_VIEW_RESOLUTION] = (struct menu_entry) { .name = "Resolution", .id = SETTINGS_RESOLUTION };
  menu[SETTINGS_VIEW_FPS] = (struct menu_entry) { .name = "FPS", .id = SETTINGS_FPS };
  menu[SETTINGS_VIEW_BITRATE] = (struct menu_entry) { .name = "Bitrate", .id = SETTINGS_BITRATE };

  // ---------
  menu[4] = (struct menu_entry) { .name = "Input", .disabled = true, .separator = true };
  menu[SETTINGS_VIEW_FRONTTOUCHSCREEN] = (struct menu_entry) { .name = "Use front touchscreen for buttons", .id = SETTINGS_FRONTTOUCHSCREEN };
  menu[SETTINGS_VIEW_DISABLE_POWERSAVE] = (struct menu_entry) { .name = "Disable power save", .id = SETTINGS_DISABLE_POWERSAVE };

  settings_loop_setup = 1;
  return display_menu(menu, 7, &settings_loop, NULL, &menu);
}

/*
 * Connect
 */

enum {
  CONNECT_PAIRUNPAIR = 13,
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
          display_error("not implemented");
        } break;
      case CONNECT_QUITAPP:
        flash_message("Quitting...");
        int ret = gs_quit_app(&server);
        if (ret == GS_OK)
          return QUIT_RELOAD;
        else
          display_error("Quitting failed: %d", ret);
        break;
      default:
        flash_message("Stream starting...");
        stream(&server, id);
        break;
    }
  }

  return 0;
}

int __connect(char *address) {
  server.address = malloc(sizeof(char)*256);
  strcpy(server.address, address);

  flash_message("Connecting to %s...", server.address);
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
    display_error("Can't connect to server %s\n", config.address);
    return;
  }

  PAPP_LIST list = NULL;
  if (gs_applist(&server, &list) != GS_OK) {
    display_error("Can't get applist!");
    return;
  }

  struct menu_entry menu[32];

  int idx = 0;

  //header
  menu[idx++] = (struct menu_entry) { .name = "Connected to the server:", .disabled = 1 };
  char server_info[256];
  sprintf(server_info, "IP: %s, GPU %s, API v%d", address, server.gpuType, server.serverMajorVersion);
  menu[idx++] = (struct menu_entry) { .name = server_info, .disabled = 1 };

  // current stream
  if (server.currentGame != 0) {
    char current_appname[256];
    char current_status[256];

    if (!get_app_name(list, server.currentGame, &current_appname)) {
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

  // app list
  if (list != NULL) {
    menu[idx++] = (struct menu_entry) { .name = "Applications", .disabled = true, .separator = true };

    while (list) {
      menu[idx++] = (struct menu_entry) { .name = list->name, .id = list->id };
      list = list->next;
    }
  }

  assert(idx < 32);
  return display_menu(menu, idx, &connect_loop, NULL, NULL);
}

void __connect_saved() {
  while (__connect(config.address) == 2);
}

void __connect_ip() {
  char ip[512];
  switch (ime_dialog(&ip, "Enter IP:", "192.168.")) {
    case 0:
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

  LiStartConnection(
      server->address,
      &config.stream,
      &connection_callbacks,
      platform_get_video(system),
      platform_get_audio(system),
      NULL,
      drFlags,
      server->serverMajorVersion
      );

  vitainput_loop();
  LiStopConnection();
}

void gui_init() {
  guilib_init();
}

void gui_loop() {
  gui_init();
  __main_menu();

  vita2d_fini();
}
