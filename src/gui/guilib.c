#include "guilib.h"

#include "../platform.h"
#include "../sdl.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>

#include <psp2/net/net.h>
#include <psp2/sysmodule.h>

#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/rtc.h>

struct menu_geom make_geom_centered(int w, int h) {
  struct menu_geom geom = {0};
  geom.x = WIDTH  / 2 - w / 2;
  geom.y = HEIGHT / 2 - h / 2;
  geom.width = w;
  geom.height = h;
  geom.total_y = geom.y + geom.height;
  return geom;
}

void draw_border(struct menu_geom geom, unsigned int border_color) {
  vita2d_draw_line(geom.x, geom.y, geom.x+geom.width, geom.y, border_color);
  vita2d_draw_line(geom.x, geom.y, geom.x, geom.y+geom.height, border_color);
  vita2d_draw_line(geom.x+geom.width, geom.y, geom.x+geom.width, geom.y+geom.height, border_color);
  vita2d_draw_line(geom.x, geom.y+geom.height, geom.x+geom.width, geom.y+geom.height, border_color);
}

void draw_text_hcentered(int x, int y, unsigned int color, char *text) {
  int width = vita2d_pgf_text_width(gui_font, 1.f, text);
  vita2d_pgf_draw_text(gui_font, x - width / 2, y, color, 1.f, text);
}

SceCtrlData ctrl_old_pad;
SceCtrlData ctrl_new_pad;

bool was_button_pressed(short id) {
  return ctrl_new_pad.buttons & id && !(ctrl_old_pad.buttons & id);
}

bool is_button_down(short id) {
  return ctrl_new_pad.buttons & id;
}

void draw_menu(struct menu_entry menu[], int total_elements, struct menu_geom geom, int cursor, int offset) {
  vita2d_draw_rectangle(geom.x, geom.y, geom.width, geom.height, 0xff101010);

  long border_color = 0xff006000;
  draw_border(geom, border_color);

  for (int i = 0, cursor_idx = 0; i < total_elements; i++) {
    long color = 0xffffffff;
    if (cursor == cursor_idx) {
      color = 0xff00ff00;
    }

    if (!menu[i].disabled) {
      cursor_idx++;
    } else {
        color = 0xffaaaaaa;
    }

    int el_x = geom.x + 10,
        el_y = geom.y + i * geom.el - offset + 10;

    if (el_y < geom.y || el_y > geom.total_y - geom.el)
      continue;

    int text_width, text_height;
    vita2d_pgf_text_dimensions(gui_font, 1.f, menu[i].name, &text_width, &text_height);

    if (menu[i].separator) {
      int border = strlen(menu[i].name) ? 7 : 0;
      int height = strlen(menu[i].name) ? text_height : geom.el / 2;
      vita2d_draw_line(
          el_x + text_width + border,
          el_y + height,
          el_x + geom.width - 10 * 2,
          el_y + height,
          border_color
          );
    }

    vita2d_pgf_draw_text(
        gui_font,
        el_x + 2,
        el_y + text_height,
        color,
        1.0f,
        menu[i].name
        );

    if (menu[i].subname) {
      int text_width = vita2d_pgf_text_width(gui_font, 1.f, menu[i].subname);
      vita2d_pgf_draw_text(
          gui_font,
          el_x + geom.width - text_width - 20,
          el_y + text_height,
          color,
          1.f,
          menu[i].subname
          );
    }
  }

  char dbg[256];
  sprintf(&dbg, "total: %d els, cur: %d, off: %d", total_elements, cursor, offset);
  vita2d_pgf_draw_text(gui_font, 0, 20, 0xffffffff, 1.f, dbg);
}

void draw_alert(char *message, struct menu_geom geom, char *buttons_captions[], int buttons_count) {
  vita2d_draw_rectangle(geom.x, geom.y, geom.width, geom.height, 0xff101010);

  long border_color = 0xff006000;
  draw_border(geom, border_color);

  char *buf = malloc(sizeof(char) * (strlen(message) + 1));
  int x_border = 10, y = 30;
  for (int i = 0, idx = 0; i < strlen(message); i++) {
    buf[idx] = message[i];
    buf[idx+1] = 0;

    if (message[i] == '\n' || vita2d_pgf_text_width(gui_font, 1.f, buf) > geom.width - x_border*2) {
      draw_text_hcentered(geom.x + geom.width / 2, y + geom.y, 0xffffffff, buf);
      y += vita2d_pgf_text_height(gui_font, 1.f, buf);
      idx = 0;
    } else {
      idx++;
    }
  }

  if (strlen(buf)) {
    draw_text_hcentered(geom.x + geom.width / 2, y + geom.y, 0xffffffff, buf);
  }

  free(buf);

  char caption[256];
  strcpy(caption, "");

  char *icons[4] = {"x", "◯", "△", "□"};
  char *default_captions[4] = {"Ok", "Cancel", "Options", "Delete"};
  for (int i = 0; i < buttons_count; i++) {
    char single_button_caption[64];
    char button_caption[256];
    if (buttons_captions && buttons_captions[i]) {
      strcpy(button_caption, buttons_captions[i]);
    } else {
      strcpy(button_caption, default_captions[i]);
    }

    sprintf(single_button_caption, "%s %s ", icons[i], button_caption);
    strcat(caption, single_button_caption);
  }

  int caption_width = vita2d_pgf_text_width(gui_font, 1.f, caption);
  vita2d_pgf_draw_text(gui_font, geom.x + geom.width - caption_width, geom.total_y - 5, 0xffffffff, 1.f, caption);
}

void gui_ctrl_begin() {
  sceCtrlPeekBufferPositive(0, &ctrl_new_pad, 1);
}

void gui_ctrl_end() {
  sceCtrlPeekBufferPositive(0, &ctrl_old_pad, 1);
}

void gui_ctrl_cursor(int *cursor_ptr, int total_elements) {
  int cursor = *cursor_ptr;
  if (was_button_pressed(SCE_CTRL_DOWN)) {
    cursor += 1;
  }

  if (was_button_pressed(SCE_CTRL_UP)) {
    cursor -= 1;
  }

  cursor = cursor < 0 ? 0 : cursor;
  cursor = cursor > total_elements - 1 ? total_elements - 1 : cursor;

  *cursor_ptr = cursor;
}

void gui_ctrl_offset(int *offset_ptr, struct menu_geom geom, int cursor) {
  int offset = *offset_ptr;

  int cursor_y = geom.y + (cursor * geom.el) - offset;
  int speed = 9;
  offset -= cursor_y < geom.y ? speed : 0;
  offset -= cursor_y > geom.total_y - geom.el * 2 ? -speed : 0;

  *offset_ptr = offset;
}

int display_menu(
    struct menu_entry menu[],
    int total_elements,
    gui_loop_callback cb,
    gui_back_callback back_cb,
    void *context
    ) {
  gui_ctrl_end();

  int offset = 0;
  int cursor = 0;
  int active_elements = 0;
  for (int i = 0; i < total_elements; i++) {
      active_elements += menu[i].disabled ? 0 : 1;
  }

  struct menu_geom geom = make_geom_centered(600, 400);
  geom.el = 25;

  int exit_code = 0;
  while (true) {
    vita2d_start_drawing();
    vita2d_clear_screen();
    draw_menu(menu, total_elements, geom, cursor, offset);

    int real_cursor = 0;
    for (int c = 0; real_cursor < total_elements; real_cursor++) {
      if (!menu[real_cursor].disabled) {
        if (cursor == c) {
          break;
        }

        c++;
      }
    }

    gui_ctrl_begin();
    gui_ctrl_cursor(&cursor, active_elements);
    gui_ctrl_offset(&offset, geom, cursor == 0 ? 0 : real_cursor);

    if (cb) {
      exit_code = cb(menu[real_cursor].id, context);
    }

    if (was_button_pressed(SCE_CTRL_CIRCLE)) {
      if (!back_cb || back_cb(context) == 0) {
        exit_code = 1;
      }
    }

    gui_ctrl_end();
    vita2d_end_drawing();
    vita2d_swap_buffers();
    sceKernelDelayThread(16 * 1000);
    if (exit_code) {
      return exit_code;
    }
  }

  return 0;
}

void display_alert(char *message, char *button_captions[], int buttons_count, gui_loop_callback cb, void *context) {
  gui_ctrl_end();

  struct menu_geom alert_geom = make_geom_centered(400, 200);
  while (true) {
    vita2d_start_drawing();
    vita2d_clear_screen();
    draw_alert(message, alert_geom, button_captions, buttons_count);

    gui_ctrl_begin();

    int result = -1;
    if (was_button_pressed(SCE_CTRL_CROSS)) {
      result = 0;
    } else if (was_button_pressed(SCE_CTRL_CIRCLE)) {
      result = 1;
    } else if (was_button_pressed(SCE_CTRL_TRIANGLE)) {
      result = 2;
    } else if (was_button_pressed(SCE_CTRL_SQUARE)) {
      result = 3;
    }

    if (cb && result != -1 && result < buttons_count) {
      switch(cb(result, context)) {
        case 1:
          gui_ctrl_end();
          return;
      }
    } else if (result == 0) {
      gui_ctrl_end();
      return;
    }

    gui_ctrl_end();
    vita2d_end_drawing();
    vita2d_swap_buffers();
  }
}

void display_error(char *format, ...) {
  char buf[0x1000];

  va_list opt;
  va_start(opt, format);
  vsnprintf(buf, sizeof(buf), format, opt);
  display_alert(buf, NULL, 1, NULL, NULL);
  va_end(opt);
}

void flash_message(char *format, ...) {
  char buf[0x1000];

  va_list opt;
  va_start(opt, format);
  vsnprintf(buf, sizeof(buf), format, opt);
  va_end(opt);

  struct menu_geom alert_geom = make_geom_centered(400, 200);
  vita2d_start_drawing();
  vita2d_clear_screen();
  vita2d_draw_rectangle(0, 0, WIDTH, HEIGHT, 0xff000000);
  draw_alert(buf, alert_geom, NULL, 0);
  vita2d_end_drawing();
  vita2d_swap_buffers();
}

void guilib_init() {
  vita2d_init();
  vita2d_set_clear_color(0xff000000);
  gui_font = vita2d_load_default_pgf();
}
