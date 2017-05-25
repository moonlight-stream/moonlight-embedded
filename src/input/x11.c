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
#include "../global.h"
#include "../loop.h"

#include <Limelight.h>

#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include <poll.h>

static Display *display;

static Atom wm_deletemessage;

static int last_x = -1, last_y = -1;
static int keyboard_modifiers;

static int x11_handler(int fd) {
  XEvent event;
  int button = 0;

  XNextEvent(display, &event);
  switch (event.type) {
  case KeyPress:
  case KeyRelease:
    if (event.xkey.keycode >= 8 && event.xkey.keycode < (sizeof(keyCodes)/sizeof(keyCodes[0]) + 8)) {
      int modifier = 0;
      switch (event.xkey.keycode) {
      case XK_Shift_R:
      case XK_Shift_L:
        modifier = MODIFIER_SHIFT;
        break;
      case XK_Alt_R:
      case XK_Alt_L:
        modifier = MODIFIER_ALT;
        break;
      case XK_Control_R:
      case XK_Control_L:
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
    }

    if (button != 0)
      LiSendMouseButtonEvent(event.type==ButtonPress ? BUTTON_ACTION_PRESS : BUTTON_ACTION_RELEASE, button);
    break;
  case MotionNotify:
    if (last_x >= 0 && last_y >= 0) {
      LiSendMouseMoveEvent(event.xmotion.x - last_x, event.xmotion.y - last_y);
    }
    last_x = event.xmotion.x;
    last_y = event.xmotion.y;
    break;
  case ClientMessage:
    if (event.xclient.data.l[0] == wm_deletemessage)
      quit();

    break;
  }
}

void x11_input_init(Display* x11_display, Window window) {
  display = x11_display;

  wm_deletemessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, window, &wm_deletemessage, 1);

  loop_add_fd(ConnectionNumber(display), x11_handler, POLLIN | POLLERR | POLLHUP);
}
