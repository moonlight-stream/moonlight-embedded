#include "ui.h"

#include "guilib.h"
#include "ime.h"

#include "ui_settings.h"
#include "ui_connect.h"
#include "ui_device.h"

#include "../config.h"
#include "../device.h"
#include "../connection.h"
#include "../video/vita.h"
#include "../input/vita.h"
#include "../power/vita.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>

#include <psp2/kernel/threadmgr.h>
#include <psp2/ctrl.h>

enum {
  MAIN_MENU_CONNECTED = 100,
  MAIN_MENU_SEARCH,
  MAIN_MENU_CONNECT_RESUME,
  MAIN_MENU_SETTINGS,
  MAIN_MENU_CONNECT,
  MAIN_MENU_CONNECT_PAIRED,
  MAIN_MENU_QUIT = 999,
};

int ui_main_menu_loop(int cursor, void *context, const input_data *input) {
  if ((input->buttons & SCE_CTRL_CROSS) == 0 || (input->buttons & SCE_CTRL_HOLD) != 0) {
    return 0;
  }
  if (cursor >= MAIN_MENU_CONNECT_PAIRED && cursor < MAIN_MENU_QUIT) {
    device_info_t *info = &known_devices.devices[cursor - MAIN_MENU_CONNECT_PAIRED];
    ui_connect_paired_device(info);
    return 2;
  }
  switch (cursor) {
    case MAIN_MENU_CONNECT:
      ui_connect_manual();
      return 2;
    case MAIN_MENU_SEARCH:
      ui_search_device();
      return 2;
    case MAIN_MENU_CONNECT_RESUME:
      ui_connect_resume();
      return 2;
    case MAIN_MENU_SETTINGS:
      ui_settings_menu();
      return 0;
    case MAIN_MENU_QUIT:
      if (connection_get_status() != LI_DISCONNECTED) {
        connection_terminate();
      }
      exit(0);
      return 0;
  }
}

int ui_main_menu_back(void *context) {
  return 1;
}

int ui_main_menu() {
  menu_entry menu[16];
  int idx = 0;

#define MENU_TITLE(NAME) \
  do { \
    menu[idx] = (menu_entry) { .name = "", .disabled = true, .color = 0xff00aa00 }; \
    strcpy(menu[idx].subname, (NAME)); \
    idx++; \
  } while (0)
#define MENU_ENTRY(ID, NAME, DISABLED) \
  do { \
    menu[idx] = (menu_entry) { .name = (NAME), .disabled = (DISABLED), .id = (ID) }; \
    idx++; \
  } while(0)
#define MENU_SEPARATOR(NAME) \
  do { \
    menu[idx] = (menu_entry) { .name = (NAME), .disabled = true, .separator = true }; \
    idx++; \
  } while(0)

  char program_info[256];
  snprintf(program_info, 256, "Moonlight v%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
  MENU_TITLE(program_info);

  char name[256] = {0};
  char addr[256] = {0};
  //if (ui_connect_connected()) {
  //  ui_connect_address(addr);
  //  sprintf(name, "Resume connection to %s", addr);
  //} else {
  //  sprintf(name, "Connect to %s", config.address ? config.address : "none");
  //}

  if (ui_connect_connected()) {
    MENU_SEPARATOR("Current connection");
    char resume_msg[256];
    ui_connect_address(addr);
    sprintf(resume_msg, "Resume connection to %s", addr);
    MENU_ENTRY(MAIN_MENU_CONNECT_RESUME, resume_msg, false);
  } else {
    MENU_SEPARATOR("Add new computer");
    MENU_ENTRY(MAIN_MENU_SEARCH, "Search devices ...", false);
    MENU_ENTRY(MAIN_MENU_CONNECT, "Add manually ...", false);

    if (known_devices.count) {
      MENU_SEPARATOR("Paired computers");
      for (int i = 0; i < known_devices.count; i++) {
        device_info_t *cur = &known_devices.devices[i];
        if (!cur->paired) {
          continue;
        }
        MENU_ENTRY(MAIN_MENU_CONNECT_PAIRED + i, cur->name, false);
      }
    }
  }

  MENU_SEPARATOR("");
  MENU_ENTRY(MAIN_MENU_SETTINGS, "Settings", false);
  MENU_ENTRY(MAIN_MENU_QUIT, "Quit", false);

  menu_geom geom = make_geom_centered(500, 200);
  return display_menu(menu, idx, &geom, &ui_main_menu_loop, &ui_main_menu_back, NULL, NULL);
}

int global_loop(int cursor, void *ctx, const input_data *input) {
  if (is_rectangle_touched(&input->touch, 0, 0, 150, 150)) {
    if (connection_get_status() == LI_MINIMIZED) {
      vitapower_config(config);
      vitainput_config(config);

      sceKernelDelayThread(500 * 1000);
      connection_resume();

      while (connection_get_status() == LI_CONNECTED) {
        sceKernelDelayThread(500 * 1000);
      }
    }
  }
}

void gui_init() {
  guilib_init(&global_loop, NULL);
}

void gui_loop() {
  gui_init();

  while (ui_main_menu() == 2);

  vita2d_fini();
}
