#include "ui.h"

#include "guilib.h"
#include "ime.h"

#include "ui_settings.h"
#include "ui_connect.h"

#include "../config.h"
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
  MAIN_MENU_CONNECT = 100,
  MAIN_MENU_CONNECT_SAVED,
  MAIN_MENU_SETTINGS,
  MAIN_MENU_QUIT
};

int ui_main_menu_loop(int cursor, void *context, const input_data *input) {
  if ((input->buttons & SCE_CTRL_CROSS) == 0 || (input->buttons & SCE_CTRL_HOLD) != 0) {
    return 0;
  }
  switch (cursor) {
    case MAIN_MENU_CONNECT:
      ui_connect_ip();
      return 2;
    case MAIN_MENU_CONNECT_SAVED:
      ui_connect_saved();
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
#define MENU_ENTRY(ID, NAME) \
  do { \
    menu[idx] = (menu_entry) { .name = (NAME), .id = (ID) }; \
    idx++; \
  } while(0)
#define MENU_SEPARATOR() \
  do { \
    menu[idx] = (menu_entry) { .name = "", .disabled = true, .separator = true }; \
    idx++; \
  } while(0)

  char program_info[256];
  snprintf(program_info, 256, "Moonlight v%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
  MENU_TITLE(program_info);

  char name[256];
  if (ui_connect_connected()) {
    char addr[256];
    ui_connect_address(addr);
    sprintf(name, "Resume connection to %s", addr);
  } else if (config.address) {
    sprintf(name, "Connect to %s", config.address ? config.address : "none");
  }

  MENU_ENTRY(MAIN_MENU_CONNECT_SAVED, name);

  MENU_ENTRY(MAIN_MENU_CONNECT, "Connect to ...");
  MENU_SEPARATOR();
  MENU_ENTRY(MAIN_MENU_SETTINGS, "Settings");
  MENU_ENTRY(MAIN_MENU_QUIT, "Quit");

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
