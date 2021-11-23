/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2017 Iwan Timmer
 *
 * Moonlight is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Moonlight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Moonlight; if not, see <http://www.gnu.org/licenses/>.
 */

#include "x11.h"
#include "keyboard.h"

#include "../loop.h"

#include <Limelight.h>

#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include <stdbool.h>
#include <stdlib.h>
#include <poll.h>

#define ACTION_MODIFIERS (MODIFIER_SHIFT|MODIFIER_ALT|MODIFIER_CTRL)
#define QUIT_KEY 0x18  /* KEY_Q */

static Display *display;
static Window window;
static XEvent event;

static Atom wm_deletemessage;

static int last_x = -1, last_y = -1;
static int keyboard_modifiers;

static const char data[1] = {0};
static Cursor cursor;
static bool grabbed = True;

static int motion_x, motion_y;

static int x11_handle_motion_notify() {
  motion_x = 0;
  motion_y = 0;
  int i;
  for (
    i = 0;
    i < 100 &&
    XPending(display) &&
    XCheckMaskEvent(display, PointerMotionMask, &event);
    ++i
  ) {
    motion_x = event.xmotion.x - last_x;
    motion_y = event.xmotion.y - last_y;
    if (abs(motion_x) > 0 || abs(motion_y) > 0) {
      if (last_x >= 0 && last_y >= 0)
        LiSendMouseMoveEvent(motion_x, motion_y);

      last_x = event.xmotion.x;
      last_y = event.xmotion.y;
    } 
  }

  if (i == 0 && grabbed) {
    XWarpPointer(display, None, window, 0, 0, 0, 0, 640, 360);
    last_x = 640;
    last_y = 360;
  }
  return i;
}

static int x11_handler(int fd) {
  int button = 0;

  while (XPending(display)) {
    XPeekEvent(display, &event);
    while (x11_handle_motion_notify() > 0) {}
    if (XCheckMaskEvent(display, ~PointerMotionMask, &event)) {
      switch (event.type) {
      case KeyPress:
      case KeyRelease:
        if (event.xkey.keycode >= 8 && event.xkey.keycode < (sizeof(keyCodes)/sizeof(keyCodes[0]) + 8)) {
          if ((keyboard_modifiers & ACTION_MODIFIERS) == ACTION_MODIFIERS && event.type == KeyRelease) {
            if (event.xkey.keycode == QUIT_KEY)
              return LOOP_RETURN;
            else {
              grabbed = !grabbed;
              XDefineCursor(display, window, grabbed ? cursor : 0);
            }
          }

          int modifier = 0;
          switch (event.xkey.keycode) {
          case 0x32:
          case 0x3e:
            modifier = MODIFIER_SHIFT;
            break;
          case 0x40:
          case 0x6c:
            modifier = MODIFIER_ALT;
            break;
          case 0x25:
          case 0x69:
            modifier = MODIFIER_CTRL;
            break;
          }

          if (modifier != 0) {
            if (event.type == KeyPress)
              keyboard_modifiers |= modifier;
            else
              keyboard_modifiers &= ~modifier;
          }

          short code = 0x80 << 8 | keyCodes[event.xkey.keycode - 8];
          LiSendKeyboardEvent(code, event.type == KeyPress ? KEY_ACTION_DOWN : KEY_ACTION_UP, keyboard_modifiers);
        }
        break;
      case ButtonPress:
      case ButtonRelease:
        switch (event.xbutton.button) {
        case Button1:
          button = BUTTON_LEFT;
          break;
        case Button2:
          button = BUTTON_MIDDLE;
          break;
        case Button3:
          button = BUTTON_RIGHT;
          break;
        case Button4:
          LiSendScrollEvent(1);
          break;
        case Button5:
          LiSendScrollEvent(-1);
          break;
        case 8:
          button = BUTTON_X1;
          break;
        case 9:
          button = BUTTON_X2;
          break;
        }

        if (button != 0)
          LiSendMouseButtonEvent(event.type==ButtonPress ? BUTTON_ACTION_PRESS : BUTTON_ACTION_RELEASE, button);
        break;
      case MotionNotify: //will never happen
        break;
      case ClientMessage:
        if (event.xclient.data.l[0] == wm_deletemessage)
          return LOOP_RETURN;

        break;
      }
    }
  }

  return LOOP_OK;
}

void x11_input_init(Display* x11_display, Window x11_window) {
  display = x11_display;
  window = x11_window;

  wm_deletemessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, window, &wm_deletemessage, 1);

  /* make a blank cursor */
  XColor dummy;
  Pixmap blank = XCreateBitmapFromData(display, window, data, 1, 1);
  cursor = XCreatePixmapCursor(display, blank, blank, &dummy, &dummy, 0, 0);
  XFreePixmap(display, blank);
  XDefineCursor(display, window, cursor);

  loop_add_fd(ConnectionNumber(display), x11_handler, POLLIN | POLLERR | POLLHUP);
}
