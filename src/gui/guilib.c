#include "guilib.h"

#include "../platform.h"

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
#include <psp2/power.h>

#define BUTTON_DELAY 150 * 1000

static gui_draw_callback gui_global_draw_callback;
static gui_loop_callback gui_global_loop_callback;

menu_geom make_geom_centered(int w, int h) {
  menu_geom geom = {0};
  geom.x = WIDTH  / 2 - w / 2;
  geom.y = HEIGHT / 2 - h / 2;
  geom.width = w;
  geom.height = h;
  geom.total_y = geom.y + geom.height;
  geom.el = 24;
  return geom;
}

void draw_border(menu_geom geom, unsigned int border_color) {
  vita2d_draw_line(geom.x, geom.y, geom.x+geom.width, geom.y, border_color);
  vita2d_draw_line(geom.x, geom.y, geom.x, geom.y+geom.height, border_color);
  vita2d_draw_line(geom.x+geom.width, geom.y, geom.x+geom.width, geom.y+geom.height, border_color);
  vita2d_draw_line(geom.x, geom.y+geom.height, geom.x+geom.width, geom.y+geom.height, border_color);
}

void draw_text_hcentered(int x, int y, unsigned int color, char *text) {
  int width = vita2d_pgf_text_width(gui_font, 1.f, text);
  vita2d_pgf_draw_text(gui_font, x - width / 2, y, color, 1.f, text);
}

static int battery_percent;
static bool battery_charging;
static SceRtcTick battery_tick;

void draw_statusbar(menu_geom geom) {
  SceRtcTick current_tick;
  sceRtcGetCurrentTick(&current_tick);
  if (current_tick.tick - battery_tick.tick > 10 * 1000 * 1000) {
    battery_percent = scePowerGetBatteryLifePercent();
    battery_charging = scePowerIsBatteryCharging();

    battery_tick = current_tick;
  }

  SceDateTime time;
  sceRtcGetCurrentClockLocalTime(&time);

  char dt_text[256];
  sprintf(dt_text, "%02d:%02d", time.hour, time.minute);
  int dt_width = vita2d_pgf_text_width(gui_font, 1.f, dt_text);
  int battery_width = 30,
      battery_height = 16,
      battery_padding = 2,
      battery_plus_height = 4,
      battery_y_offset = 4,
      battery_charge_width = (float) battery_percent / 100 * battery_width;
  unsigned int battery_color = battery_charging ? 0xff99ffff : (battery_percent < 20 ? 0xff0000ff : 0xff00ff00);

  vita2d_pgf_draw_text(gui_font, geom.x + geom.width - dt_width - battery_width - 5, geom.y - 5, 0xffffffff, 1.f, dt_text);

  vita2d_draw_rectangle(
      geom.x + geom.width - battery_width,
      geom.y - battery_height - battery_y_offset,
      battery_width,
      battery_height,
      0xffffffff);

  vita2d_draw_rectangle(
      geom.x + geom.width - battery_width - battery_padding,
      geom.y - battery_y_offset - ((float) battery_height / 2 + (float) battery_plus_height / 2),
      battery_padding,
      battery_plus_height,
      0xffffffff
      );

  vita2d_draw_rectangle(
      geom.x + geom.width - battery_width + battery_padding,
      geom.y - battery_height + battery_padding - battery_y_offset,
      battery_width - battery_padding * 2,
      battery_height - battery_padding * 2,
      0xff000000);

  vita2d_draw_rectangle(
      geom.x + geom.width - battery_charge_width + battery_padding,
      geom.y - battery_height + battery_padding - battery_y_offset,
      battery_charge_width - battery_padding * 2,
      battery_height - battery_padding * 2,
      battery_color);
}

SceTouchData touch_data;
SceCtrlData ctrl_new_pad;

static SceRtcTick button_current_tick, button_until_tick;
bool was_button_pressed(short id) {
  sceRtcGetCurrentTick(&button_current_tick);

  if (ctrl_new_pad.buttons & id) {
    if (sceRtcCompareTick(&button_current_tick, &button_until_tick) > 0) {
      sceRtcTickAddMicroseconds(&button_until_tick, &button_current_tick, BUTTON_DELAY);
      return true;
    }
  }

  return false;
}

bool is_button_down(short id) {
  return ctrl_new_pad.buttons & id;
}

#define lerp(value, from_max, to_max) ((((value*10) * (to_max*10))/(from_max*10))/10)
bool is_rectangle_touched(const SceTouchData *touch, int lx, int ly, int rx, int ry) {
  for (int i = 0; i < touch->reportNum; i++) {
    int x = lerp(touch->report[i].x, 1919, WIDTH);
    int y = lerp(touch->report[i].y, 1087, HEIGHT);
    if (x < lx || x > rx || y < ly || y > ry) continue;
    return true;
  }

  return false;
}

void draw_menu(menu_entry menu[], int total_elements, menu_geom geom, int cursor, int offset) {
  vita2d_draw_rectangle(geom.x, geom.y, geom.width, geom.height, 0x10ffffff);

  long border_color = 0xff006000;
  draw_border(geom, border_color);
  draw_statusbar(geom);

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

    if (menu[i].color) {
      color = menu[i].color;
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
          0xffaaaaaa
          );
    }

    if (menu[i].name) {
      vita2d_pgf_draw_text(
          gui_font,
          el_x + 2,
          el_y + text_height,
          color,
          1.0f,
          menu[i].name
          );
    }

    int right_x_offset = 20;
    if (menu[i].suffix) {
      int text_width = vita2d_pgf_text_width(gui_font, 1.f, menu[i].suffix);
      vita2d_pgf_draw_text(
          gui_font,
          el_x + geom.width - text_width - right_x_offset,
          el_y + text_height,
          color,
          1.f,
          menu[i].suffix
          );

      right_x_offset += text_width + 10;
    }

    if (menu[i].subname) {
      int text_width = vita2d_pgf_text_width(gui_font, 1.f, menu[i].subname);
      vita2d_pgf_draw_text(
          gui_font,
          el_x + geom.width - text_width - right_x_offset,
          el_y + text_height,
          color,
          1.f,
          menu[i].subname
          );
    }
  }
}

void draw_alert(char *message, menu_geom geom, char *buttons_captions[], int buttons_count) {
  vita2d_draw_rectangle(geom.x, geom.y, geom.width, geom.height, 0x10ffffff);

  long border_color = 0xff006000;
  draw_border(geom, border_color);

  char *buf = malloc(sizeof(char) * (strlen(message) + 1));
  int top_padding = 30;
  int x_border = 10, y = top_padding;
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
    if (y == top_padding) {
      int text_height = vita2d_pgf_text_height(gui_font, 1.f, buf);
      y = geom.height / 2 - text_height / 2;
    }

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
  vita2d_pgf_draw_text(gui_font, geom.x + geom.width - caption_width, geom.total_y - 10, 0xffffffff, 1.f, caption);
}

void ui_start() {
  vita2d_start_drawing();
  vita2d_clear_screen();
}

void ui_end() {
  vita2d_end_drawing();
  vita2d_wait_rendering_done();
  vita2d_swap_buffers();
}

int read_buttons() {
    SceCtrlData pad = {0};
    static int old;
    static int hold_times;
    int curr, btn;

    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);
    sceCtrlPeekBufferPositive(0, &pad, 1);

    if (pad.ly < 0x10) {
        pad.buttons |= SCE_CTRL_UP;
    } else if (pad.ly > 0xef) {
        pad.buttons |= SCE_CTRL_DOWN;
    }
    curr = pad.buttons;
    btn = pad.buttons & ~old;
    if (curr && old == curr) {
        hold_times += 1;
        if (hold_times >= 10) {
            btn = curr;
            hold_times = 8;
            btn |= SCE_CTRL_HOLD;
        }
    } else {
        hold_times = 0;
        old = curr;
    }
    return btn;
}

int display_menu(menu_entry menu[], int total_elements, menu_geom *geom_ptr,
                 gui_loop_callback cb, gui_back_callback back_cb,
                 gui_draw_callback draw_callback,
                 void *context) {
  ui_end();

  int offset = 0;
  int cursor = 0;

  menu_geom geom;

  if (!geom_ptr) {
    geom = make_geom_centered(600, 400);
    geom.el = 24;
  } else {
    geom = *geom_ptr;
  }

  int tick_number = 0;
  int exit_code = 0;

  while (true) {
    int active_elements = 0;
    for (int i = 0; i < total_elements; i++) {
        active_elements += menu[i].disabled ? 0 : 1;
    }

    ui_start();
    tick_number++;

    if (tick_number > 3) {
      if (draw_callback) {
        draw_callback();
      }
      if (gui_global_draw_callback) {
        gui_global_draw_callback();
      }
    }

    draw_menu(menu, total_elements, geom, cursor, offset);

    int real_cursor = 0;

    for (int c = 0; real_cursor < total_elements; real_cursor++) {
      if (menu[real_cursor].disabled) {
        continue;
      }
      if (cursor == c) {
        break;
      }
      c++;
    }

    // select item
    input_data input = {0};
    input.buttons = read_buttons();
    sceTouchPeek(SCE_TOUCH_PORT_FRONT, &input.touch, 1);
    if (input.buttons & SCE_CTRL_DOWN) {
      cursor += 1;
    }
    if (input.buttons & SCE_CTRL_UP) {
      cursor -= 1;
    }
    cursor = cursor < 0 ? 0 : cursor;
    cursor = cursor > active_elements - 1 ? active_elements - 1 : cursor;

    int cursor_y = geom.y + ((cursor == 0 ? 0 : real_cursor) * geom.el) - offset;
    offset -= cursor_y < geom.y ? 8 : 0;
    offset -= cursor_y > geom.total_y - geom.el * 2 ? -8 : 0;

    if (cb) {
      exit_code = cb(menu[real_cursor].id, context, &input);
      if (exit_code) {
        goto error;
      }
    }

    if (gui_global_loop_callback) {
      gui_global_loop_callback(menu[real_cursor].id, context, &input);
    }

    if (input.buttons & SCE_CTRL_CIRCLE && (input.buttons & SCE_CTRL_HOLD) == 0) {
      if (!back_cb || back_cb(context) == 0) {
        exit_code = 1;
        goto error;
      }
    }

    ui_end();
  }

  return 0;

error:
  ui_end();
  return exit_code;
}


void display_alert(char *message, char *button_captions[], int buttons_count,
                   gui_loop_callback cb, void *context) {

  menu_geom alert_geom = make_geom_centered(400, 200);

  while (true) {
    ui_start();

    draw_alert(message, alert_geom, button_captions, buttons_count);

    input_data input = {0};
    input.buttons = read_buttons();
    sceTouchPeek(SCE_TOUCH_PORT_FRONT, &input.touch, 1);

    int result = -1;

    if (input.buttons & SCE_CTRL_HOLD) {
      ui_end();
      continue;
    }

    if (input.buttons & SCE_CTRL_CROSS) {
      result = 0;
    } else if (input.buttons & SCE_CTRL_CIRCLE) {
      result = 1;
    } else if (input.buttons & SCE_CTRL_TRIANGLE) {
      result = 2;
    } else if (input.buttons & SCE_CTRL_SQUARE) {
      result = 3;
    }

    if (cb && result != -1 && result < buttons_count) {
      switch(cb(result, context, &input)) {
        case 1:
          return;
      }
    } else if (result == 0) {
      return;
    }

    ui_end();
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

  ui_end();

  menu_geom alert_geom = make_geom_centered(400, 200);
  ui_start();

  vita2d_draw_rectangle(0, 0, WIDTH, HEIGHT, 0xff000000);
  draw_alert(buf, alert_geom, NULL, 0);

  ui_end();
}

void drw() {
  vita2d_draw_rectangle(0, 0, 150, 150, 0xffffffff);
}

void guilib_init(gui_loop_callback global_loop_cb, gui_draw_callback global_draw_cb) {
  vita2d_init();
  vita2d_set_clear_color(0xff000000);
  gui_font = vita2d_load_default_pgf();

  gui_global_draw_callback = global_draw_cb;
  gui_global_loop_callback = global_loop_cb;
}
