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

#include <psp2/kernel/threadmgr.h>
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
  int ret = gs_start_app(server, &config.stream, appId, config.sops, config.localaudio, true);
  if (ret < 0) {
    if (ret == GS_NOT_SUPPORTED_4K)
      display_error("Server doesn't support 4K\n");
    else if (ret == GS_NOT_SUPPORTED_MODE)
      display_error("Server doesn't support %dx%d (%d fps)\n", config.stream.width, config.stream.height, config.stream.fps);
    else
      display_error("Errorcode starting app: %d\n", ret);

    return;
  }

  enum platform system = VITA;
  int drFlags = 0;

  if (config.fullscreen)
    drFlags |= DISPLAY_FULLSCREEN;

  DECODER_RENDERER_CALLBACKS *video_callback = platform_get_video(system);
  if (config.enable_ref_frame_invalidation) {
    video_callback->capabilities |= CAPABILITY_REFERENCE_FRAME_INVALIDATION_AVC;
  } else {
    video_callback->capabilities &= ~CAPABILITY_REFERENCE_FRAME_INVALIDATION_AVC;
  }

  ret = LiStartConnection(&server->serverInfo, &config.stream, &connection_callbacks,
                          video_callback, platform_get_audio(system),
                          NULL, drFlags, NULL, 0);

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

    display_error("Failed to start stream: error code %d\nFailed stage: %s\n(error code %d)",
                  ret, stage, connection_failed_stage_code);
    return;
  }
}

static SERVER_DATA server;
static PAPP_LIST server_applist;
static int pos[2];

enum {
  CONNECT_PAIRUNPAIR = 13,
  CONNECT_DISCONNECT,
  CONNECT_QUITAPP
};

#define QUIT_RELOAD 2

int ui_connect_loop(int id, void *context, const input_data *input) {
  int status = connection_get_status();

  if (status == LI_DISCONNECTED) {
      goto disconnect;
  }

  menu_entry *menu = context;
  for (int i = pos[0]; i < pos[1]; i += 1) {
    menu[i].disabled = (server.currentGame != 0);
  }

  if ((input->buttons & SCE_CTRL_CROSS) == 0 || input->buttons & SCE_CTRL_HOLD) {
    return 0;
  }

  int ret;

  switch (id) {
    case CONNECT_PAIRUNPAIR:
      if (server.paired) {
        flash_message("Unpairing...");
        ret = gs_unpair(&server);
        if (ret == GS_OK) {
          if (connection_terminate()) {
            display_error("Reconnect failed: %d", -1);
            return 0;
          }
          return QUIT_RELOAD;
        }
        display_error("Unpairing failed: %d", ret);
        return 0;
      }

      char pin[5];
      char message[256];
      sprintf(pin, "%d%d%d%d",
              (int)rand() % 10, (int)rand() % 10, (int)rand() % 10, (int)rand() % 10);
      flash_message("Please enter the following PIN\non the target PC:\n\n%s", pin);

      ret = gs_pair(&server, &pin[0]);
      if (ret == 0) {
        connection_paired();
        if (connection_terminate()) {
          display_error("Reconnect failed: %d", -2);
          return 0;
        }
        return QUIT_RELOAD;
      }
      display_error("Pairing failed: %d", ret);
      return 0;

    case CONNECT_DISCONNECT:
      goto disconnect;

    case CONNECT_QUITAPP:
      flash_message("Quitting...");
      ret = gs_quit_app(&server);
      if (ret == GS_OK) {
        connection_paired();
        server.currentGame = 0;
        return QUIT_RELOAD;
      }
      display_error("Quitting failed: %d", ret);
      return 0;

    default:
      vitapower_config(config);
      vitainput_config(config);

      switch (status) {
        case LI_MINIMIZED:
          if (server.currentGame == id) {
            sceKernelDelayThread(500 * 1000);
            connection_resume();
            break;
          }
          // TODO: stop previous stream
        case LI_PAIRED:
          flash_message("Stream starting...");
          ui_connect_stream(&server, id);
          break;
      }

mainloop:
      while (connection_get_status() == LI_CONNECTED) {
        sceKernelDelayThread(500 * 1000);
      }

      int status = connection_get_status();

      if (status == LI_DISCONNECTED) {
          goto disconnect;
      }

      return QUIT_RELOAD;
  }

disconnect:
  flash_message("Disconnecting...");
  connection_terminate();
  sceKernelDelayThread(1000 * 1000);
  return 1;
}

int ui_connect(char *address) {
  int ret;
  if (!connection_is_ready()) {
    flash_message("Connecting to:\n %s...", address);
    ret = gs_init(&server, address, config.key_dir);
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
    } else if (ret == GS_ERROR) {
      display_error("Gamestream error: %s\n", gs_error);
      return 0;
    } else if (ret != GS_OK) {
      display_error("Can't connect to server\n%s", address);
      return 0;
    }

    connection_reset();
  }

  int app_count = 0;
  if (server.paired) {
    ret = gs_applist(&server, &server_applist);
    if (ret != GS_OK) {
      display_error("Can't get applist!\n%d\n%s", ret, gs_error);
      return 0;
    }

    if (server_applist != NULL) {
      PAPP_LIST list = server_applist;
      while (list) {
        list = list->next;
        app_count += 1;
      }
    }
  }

  // current menu = 11 + app_count. but little more alloc ;)
  struct menu_entry menu[app_count + 16];

  int idx = 0;

#define MENU_CATEGORY(NAME) \
  do { \
    menu[idx] = (menu_entry) { .name = (NAME), .disabled = true, .separator = true }; \
    idx++; \
  } while (0)
#define MENU_ENTRY(ID, NAME) \
  do { \
    menu[idx] = (menu_entry) { .name = (NAME), .id = (ID) }; \
    idx++; \
  } while(0)
#define MENU_MESSAGE(MESSAGE, COLOR) \
  do { \
    menu[idx] = (menu_entry) { .name = (MESSAGE), .disabled = true, .color = (COLOR) }; \
    idx++; \
  } while(0)

  //header
  //MENU
  MENU_MESSAGE("Connected to the server:", 0xffffffff);
  char server_info[256];
  snprintf(server_info, 256,
           "IP: %s, GPU %s, GFE %s",
           address,
           server.gpuType,
           server.serverInfo.serverInfoGfeVersion);

  MENU_MESSAGE(server_info, 0xffffffff);
  MENU_MESSAGE("", 0);

  if (!server.paired) {
    // pairing
    MENU_CATEGORY("Not paired");
    MENU_ENTRY(CONNECT_PAIRUNPAIR, "Pair");

    MENU_ENTRY(CONNECT_DISCONNECT, "Disconnect");
  } else {
    // current stream
    if (server.currentGame != 0) {
      char current_appname[256];
      char current_status[256];

      if (!get_app_name(server_applist, server.currentGame, current_appname)) {
        strcpy(current_appname, "unknown");
      }
      sprintf(current_status, "Streaming %s", current_appname);

      MENU_CATEGORY(current_status);
      MENU_ENTRY(server.currentGame, "Resume");
      MENU_ENTRY(CONNECT_QUITAPP, "Quit");
    }

    // pairing
    MENU_CATEGORY("Paired");
    connection_paired();
    // FIXME: unpair not work
    // MENU_ENTRY(CONNECT_PAIRUNPAIR, "Unpair");

    MENU_ENTRY(CONNECT_DISCONNECT, "Disconnect");

    // app list
    if (server_applist != NULL) {
      MENU_CATEGORY("Applications");

      pos[0] = idx;

      PAPP_LIST list = server_applist;
      while (list) {
        MENU_ENTRY(list->id, list->name);
        list = list->next;
      }

      pos[1] = idx;
    } else {
      pos[0] = -1;
    }
  }

  return display_menu(menu, idx, NULL, &ui_connect_loop, NULL, NULL, &menu);
}

void ui_connect_saved() {
  while (ui_connect(config.address) == QUIT_RELOAD);
}

void ui_connect_ip() {
  char ip[512];
  switch (ime_dialog(ip, "Enter IP:", "192.168.")) {
    case 0:
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
  return connection_is_ready();
}

void ui_connect_address(char *addr) {
  strcpy(addr, server.serverInfo.address);
}
