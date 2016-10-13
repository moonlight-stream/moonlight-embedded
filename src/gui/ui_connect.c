#include "ui_connect.h"

#include "guilib.h"
#include "ime.h"

#include "ui_settings.h"

#include "../connection.h"
#include "../configuration.h"
#include "../video.h"
#include "../config.h"

#include "client.h"
#include "discover.h"
#include "../platform.h"

#include "../power/vita.h"
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

void ui_connect_stream(PSERVER_DATA server, int appId) {
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
      &server->serverInfo,
      &config.stream,
      &connection_callbacks,
      platform_get_video(system),
      platform_get_audio(system),
      NULL,
      drFlags
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

static SERVER_DATA server;
static PAPP_LIST server_applist;
static bool server_connected;

enum {
  CONNECT_PAIRUNPAIR = 13,
  CONNECT_DISCONNECT,
  CONNECT_QUITAPP
};

#define QUIT_RELOAD 2

int ui_connect_loop(int id, void *context) {
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
          ui_connect_stream(&server, id);
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

int ui_connect(char *address) {
  if (!server_connected) {
    flash_message("Connecting to:\n %s...", address);
    int ret = gs_init(&server, address, config.key_dir);
    if (ret == GS_OUT_OF_MEMORY) {
      display_error("Not enough memory");
      return 0;
    } else if (ret == GS_INVALID) {
      display_error("Invalid data received from server: %s\n", address, gs_error);
      return 0;
    } else if (ret == GS_UNSUPPORTED_VERSION) {
      if (!config.unsupported_version) {
        display_error("Unsupported version: %s\n", gs_error);
        return 0;
      }
    } else if (ret != GS_OK) {
      display_error("Can't connect to server\n%s", address);
      return 0;
    }

    if (gs_applist(&server, &server_applist) != GS_OK) {
      display_error("Can't get applist!");
      return 0;
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
  return display_menu(menu, idx, NULL, &ui_connect_loop, NULL, NULL, NULL);
}

void ui_connect_saved() {
  while (ui_connect(config.address) == 2);
}

void ui_connect_ip() {
  char ip[512];
  switch (ime_dialog(&ip, "Enter IP:", "192.168.")) {
    case 0:
      server_connected = false;
      if (config.address)
        free(config.address);
      config.address = malloc(sizeof(char) * strlen(ip));
      strcpy(config.address, ip);
      ui_settings_save_config();
      ui_connect_saved();
      break;
    default:
      return;
  }
}

bool ui_connect_connected() {
  return server_connected;
}

void ui_connect_address(char *addr) {
  strcpy(addr, server.address);
}
