#include "ui.h"
#include "guilib.h"
#include "ime.h"
#include "ui_settings.h"

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
        settings_menu();
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
  char name[256];
  if (server_connected) {
    sprintf(name, "Resume connection to %s", server.address);
    menu[idx++] = (struct menu_entry) { .name = name, .id = MAIN_MENU_CONNECT_SAVED };
  } else if (config.address) {
    sprintf(name, "Connect to %s", config.address ? config.address : "none");
    menu[idx++] = (struct menu_entry) { .name = name, .id = MAIN_MENU_CONNECT_SAVED };
  }

  menu[idx++] = (struct menu_entry) { .name = "Connect to ...", .id = MAIN_MENU_CONNECT };
  menu[idx++] = (struct menu_entry) { .name = "", .disabled = true, .separator = true };
  menu[idx++] = (struct menu_entry) { .name = "Settings", .id = MAIN_MENU_SETTINGS };
  menu[idx++] = (struct menu_entry) { .name = "Quit", .id = MAIN_MENU_QUIT };

  struct menu_geom geom = make_geom_centered(500, 200);
  return display_menu(menu, idx, &geom, &main_menu_loop, &main_menu_back, NULL, NULL);
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
        flash_message("Disconnecting...");
        server_connected = false;
        connection_terminate();
        sceKernelDelayThread(1000 * 1000);
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
          stream(&server, id);
        } else if (connection_get_status() == LI_MINIMIZED) {
          sceKernelDelayThread(500 * 1000);
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

  struct menu_entry menu[48];

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

  assert(idx < 48);
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
    server->currentGame = appId;
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
