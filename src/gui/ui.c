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
        gui_ctrl_end();
        __connect_ip();
        break;
      case MAIN_MENU_CONNECT_SAVED:
        gui_ctrl_end();
        __connect_saved();
        break;
      case MAIN_MENU_SETTINGS:
        gui_ctrl_end();
        __settings();
        break;
      case MAIN_MENU_QUIT:
        exit(0);
        break;
      case 200:
        if (true) {
          gui_ctrl_end();
          char *captions[] = {"foo", "bar", 0, 0};
          display_alert("dis is ALERT", captions, 4, 0);
          display_error("dis is error");
        }
        break;
    }
  }

  return 0;
}

void __main_menu() {
  struct menu_entry menu[16];
  menu[0] = (struct menu_entry) { .name = "connect to ip", .id = MAIN_MENU_CONNECT };
  char prev[256];
  sprintf(prev, "connect to %s", config.address ? config.address : "none");

  menu[1] = (struct menu_entry) { .name = prev, .id = MAIN_MENU_CONNECT_SAVED };
  menu[2] = (struct menu_entry) { .name = "", .disabled = true, .separator = true };
  menu[3] = (struct menu_entry) { .name = "settings", .id = MAIN_MENU_SETTINGS };
  menu[4] = (struct menu_entry) { .name = "test", .id = 200 };
  menu[5] = (struct menu_entry) { .name = "quit", .id = MAIN_MENU_QUIT };

  display_menu(menu, 6, &main_menu_loop, 0);
}

enum {
  CONNECT_START = 100,
  CONNECT_PAIRUNPAIR
};


/*
 * Connect
 */

int connect_loop(int cursor, void *context) {
  if (was_button_pressed(SCE_CTRL_CROSS)) {
    enum platform system = VITA;

    switch (cursor) {
      case CONNECT_START:
        gui_ctrl_end();
        flash_message("Stream starting...");
        stream(&server, &config, system);
        break;
      case CONNECT_PAIRUNPAIR:
        gui_ctrl_end();
        display_error("not implemented");
        break;
    }
  }

  return 0;
}

void __connect(char *address) {
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

  struct menu_entry menu[16];
  menu[0] = (struct menu_entry) { .name = "Connected to the server:", .disabled = 1 };
  menu[1] = (struct menu_entry) { .name = address, .disabled = 1 };

  char server_stats[256];
  sprintf(server_stats, "%sPAIRED, %s", server.paired ? "" : "NOT", server.currentGame != 0 ? "STREAMING" : "NOT STREAMING");
  menu[2] = (struct menu_entry) { .name = server_stats, .disabled = 1 };
  menu[3] = (struct menu_entry) { .name = "", .disabled = true, .separator = true };
  menu[4] = (struct menu_entry) { .name = "Start streaming", .disabled = !server.paired, .id = CONNECT_START };

  if (server.paired) {
    menu[5] = (struct menu_entry) { .name = "Unpair", .id = CONNECT_PAIRUNPAIR };
  } else {
    menu[5] = (struct menu_entry) { .name = "Pair", .id = CONNECT_PAIRUNPAIR };
  }

  display_menu(menu, 6, &connect_loop, 0);
}

void __connect_saved() {
  __connect(config.address);
}

void __connect_ip() {
  char ip[256];
  switch (ime_dialog(&ip)) {
    case 0:
      gui_ctrl_end();
      strcpy(config.address, ip);
      __connect_saved();
      break;
    default:
      return;
  }
}

/*
 * Settings
 */

enum {
  RESOLUTION = 100
};

int settings_loop(int id, void *context) {
  switch (id) {
    case RESOLUTION:
      if (true) {

        int count = 3;
        char *resolution[count];
        resolution[0] = "960x544";
        resolution[1] = "1280x720";
        resolution[2] = "1920x1080";
        char current[256];
        sprintf(current, "%dx%d", config.stream.width, config.stream.height);

        bool left = was_button_pressed(SCE_CTRL_LEFT), right = was_button_pressed(SCE_CTRL_RIGHT);
        if (left || right) {
          gui_ctrl_end();
          int i = 0;
          for (; i < count; i++) {
            if (strcmp(current, resolution[i]) == 0) {
              i += right ? 1 : (left ? -1 : 0);
              break;
            }
          }

          if (i >= count || i < 0) {
            break;
          } else {
            switch (i) {
              case 0:
                config.stream.width = 960;
                config.stream.height = 544;
                break;
              case 1:
                config.stream.width = 1280;
                config.stream.height = 720;
                break;
              case 2:
                config.stream.width = 1920;
                config.stream.height = 1080;
                break;
            }
          }

          display_error("%dx%d", config.stream.width, config.stream.height);
        }

      }
      break;
  }

  return 0;
}

void __settings() {
  struct menu_entry menu[16];
  menu[0] = (struct menu_entry) { .name = "Stream", .disabled = true, .separator = true};
  menu[1] = (struct menu_entry) { .name = "Resolution", .id = RESOLUTION };
  menu[2] = (struct menu_entry) { .name = "FPS" };
  menu[3] = (struct menu_entry) { .name = "Bitrate" };

  menu[3] = (struct menu_entry) { .name = "Input", .disabled = true, .separator = true };
  menu[4] = (struct menu_entry) { .name = "Use front touchscreen for buttons" };
  menu[5] = (struct menu_entry) { .name = "Disable power save" };

  display_menu(menu, 6, &settings_loop, 0);
}

/*
 * Actions
 */

int get_app_id(PSERVER_DATA server, const char *name) {
  PAPP_LIST list = NULL;
  if (gs_applist(server, &list) != GS_OK) {
    printf("Can't get app list\n");
    return -1;
  }

  while (list != NULL) {
    if (strcmp(list->name, name) == 0)
      return list->id;

    list = list->next;
  }
  return -1;
}

void stream(PSERVER_DATA server, PCONFIGURATION config, enum platform system) {
  int appId = get_app_id(server, config->app);
  if (appId<0) {
    return display_error("Can't find app %s\n", config->app);
  }

  int ret = gs_start_app(server, &config->stream, appId, config->sops, config->localaudio);
  if (ret < 0) {
    if (ret == GS_NOT_SUPPORTED_4K)
      display_error("Server doesn't support 4K\n");
    else
      display_error("Errorcode starting app: %d\n", ret);

    return;
  }

  int drFlags = 0;
  if (config->fullscreen)
    drFlags |= DISPLAY_FULLSCREEN;
  if (config->forcehw)
    drFlags |= FORCE_HARDWARE_ACCELERATION;

  LiStartConnection(
      server->address,
      &config->stream,
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
