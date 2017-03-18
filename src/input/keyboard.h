/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2015 Iwan Timmer
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

static const short keyCodes[] = {
  0, //VK_RESERVED
  0x1B, //VK_ESCAPE
  0x31, //VK_1
  0x32, //VK_2
  0x33, //VK_3
  0x34, //VK_4
  0x35, //VK_5
  0x36, //VK_6
  0x37, //VK_7
  0x38, //VK_8
  0x39, //VK_9
  0x30, //VK_0
  0xBD, //VK_MINUS
  0xBB, //VK_EQUALS
  0x08, //VK_BACK_SPACE
  0x09, //VK_TAB
  0x51, //VK_Q
  0x57, //VK_W
  0x45, //VK_E
  0x52, //VK_R
  0x54, //VK_T
  0x59, //VK_Y
  0x55, //VK_U
  0x49, //VK_I
  0x4F, //VK_O
  0x50, //VK_P
  0xDB, //VK_BRACELEFT
  0xDD, //VK_BRACERIGHT
  0x0D, //VK_ENTER
  0x11, //VK_CONTROL Left control
  0x41, //VK_A
  0x53, //VK_S
  0x44, //VK_D
  0x46, //VK_F
  0x47, //VK_G
  0x48, //VK_H
  0x4A, //VK_J
  0x4B, //VK_K
  0x4C, //VK_L
  0xBA, //VK_SEMICOLON
  0xDE, //VK_APOSTROPHE
  0xC0, //VK_GRAVE
  0x10, //VK_SHIFT Left shift
  0xDC, //VK_BACK_SLASH
  0x5A, //VK_Z
  0x58, //VK_X
  0x43, //VK_C
  0x56, //VK_V
  0x42, //VK_B
  0x4E, //VK_N
  0x4D, //VK_M
  0xBC, //VK_COMMA
  0xBE, //VK_DOT
  0xBF, //VK_SLASH
  0x10, //VK_SHIFT Right shift
  0x6A, //VK_KPASTERISK
  0x12, //VK_ALT Left alt
  0x20, //VK_SPACE
  0x14, //VK_CAPS_LOCK
  0x70, //VK_F1
  0x71, //VK_F2
  0x72, //VK_F3
  0x73, //VK_F4
  0x74, //VK_F5
  0x75, //VK_F6
  0x76, //VK_F7
  0x77, //VK_F8
  0x78, //VK_F9
  0x79, //VK_F10
  0x90, //VK_NUM_LOCK
  0x91, //VK_SCROLL_LOCK
  0x67, //VK_NUMPAD7
  0x68, //VK_NUMPAD8
  0x69, //VK_NUMPAD9
  0x6D, //VK_NUMPAD_MINUS
  0x64, //VK_NUMPAD4
  0x65, //VK_NUMPAD5
  0x66, //VK_NUMPAD6
  0x6B, //VK_NUMPADPLUS
  0x61, //VK_NUMPAD1
  0x62, //VK_NUMPAD2
  0x63, //VK_NUMPAD3
  0x60, //VK_NUMPAD0
  0x6E, //KeyEvent.VK_NUMPADDOT
  0,
  0, //KeyEvent.VK_ZENKAKUHANKAKU
  0, //KeyEvent.VK_102ND
  0x7A, //VK_F11
  0x7B, //VK_F12
  0, //KeyEvent.VK_RO
  0xF1, //VK_KATAKANA
  0xF2, //VK_HIRAGANA
  0, //VK_HENKAN
  0, //VK_KATAKANAHIRAGANA
  0, //VK_MUHENKAN
  0, //VK_KPJPCOMMA
  0x0D, //VK_KPENTER
  0x11, //VK_CONTROL Right ctrl
  0x6F, //VK_KPSLASH
  0x2C, //VK_SYSRQ
  0x12, //VK_ALT Right alt
  0, //KeyEvent.VK_LINEFEED
  0x24, //VK_HOME
  0x26, //VK_UP
  0x21, //VK_PAGE_UP
  0x25, //VK_LEFT
  0x27, //VK_RIGHT
  0x23, //VK_END
  0x28, //VK_DOWN
  0x22, //VK_PAGE_DOWN
  0x9B, //VK_INSERT
  0x2E, //VK_DELETE
  0, //VK_MACRO
  0, //VK_MUTE
  0, //VK_VOLUMEDOWN
  0, //VK_VOLUMEUP
  0, //VK_POWER SC System Power Down
  0, //VK_KPEQUAL
  0, //VK_KPPLUSMINUS
  0x13, //VK_PAUSE
  0, //VK_SCALE AL Compiz Scale (Expose)
};
