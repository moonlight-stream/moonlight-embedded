#include "ui.h"
#include "guilib.h"
#include "ime.h"
#include "ui_settings.h"
#include "ui_connect.h"

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

/*
 * Main menu
 */

enum {
  MAIN_MENU_CONNECT = 100,
  MAIN_MENU_CONNECT_SAVED,
  MAIN_MENU_SETTINGS,
  MAIN_MENU_QUIT
};

int ui_main_menu_loop(int cursor, void *context) {
  if (was_button_pressed(SCE_CTRL_CROSS)) {
    switch (cursor) {
      case MAIN_MENU_CONNECT:
        ui_connect_ip();
        return 2;
        break;
      case MAIN_MENU_CONNECT_SAVED:
        ui_connect_saved();
        return 2;
        break;
      case MAIN_MENU_SETTINGS:
        ui_settings_menu();
        break;
      case MAIN_MENU_QUIT:
        exit(0);
        break;
    }
  }

  return 0;
}

int ui_main_menu_back(void *context) {
  return 1;
}

int ui_main_menu() {
  struct menu_entry menu[16];
  int idx = 0;

  menu[idx++] = (struct menu_entry) { .name = "", .subname = "Moonlight Alpha", .disabled = true, .color = 0xff00aa00 };
  char name[256];
  if (ui_connect_connected()) {
    char addr[256];
    ui_connect_address(&addr);
    sprintf(name, "Resume connection to %s", addr);
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
  return display_menu(menu, idx, &geom, &ui_main_menu_loop, &ui_main_menu_back, NULL, NULL);
}

void gui_init() {
  guilib_init();
}

void gui_loop() {
  gui_init();

  while (ui_main_menu() == 2);

  vita2d_fini();
}
