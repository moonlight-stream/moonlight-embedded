/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2015-2017 Iwan Timmer
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

#ifdef HAVE_LIBCEC

#include <Limelight.h>

#include <ceccloader.h>

#define KEY_LEFT 0x25
#define KEY_UP 0x26
#define KEY_RIGHT 0x27
#define KEY_DOWN 0x28
#define KEY_ENTER 0x0D
#define KEY_TAB 0x09
#define KEY_ESC 0x1B

static libcec_configuration g_config;
static char                 g_strPort[50] = { 0 };
static libcec_interface_t   g_iface;
static ICECCallbacks        g_callbacks;

static void on_cec_keypress(void* userdata, const cec_keypress* key) {
  char value;
  switch (key->keycode) {
    case CEC_USER_CONTROL_CODE_UP:
      value = KEY_UP;
      break;
    case CEC_USER_CONTROL_CODE_DOWN:
      value = KEY_DOWN;
      break;
    case CEC_USER_CONTROL_CODE_LEFT:
      value = KEY_LEFT;
      break;
    case CEC_USER_CONTROL_CODE_RIGHT:
      value = KEY_RIGHT;
      break;
    case CEC_USER_CONTROL_CODE_ENTER:
    case CEC_USER_CONTROL_CODE_SELECT:
      value = KEY_ENTER;
      break;
    case CEC_USER_CONTROL_CODE_ROOT_MENU:
      value = KEY_TAB;
      break;
    case CEC_USER_CONTROL_CODE_AN_RETURN:
    case CEC_USER_CONTROL_CODE_EXIT:
      value = KEY_ESC;
      break;
    default:
      value = 0;
      break;
  }
  
  if (value != 0) {
    short code = 0x80 << 8 | value;
    LiSendKeyboardEvent(code, (key->duration > 0)?KEY_ACTION_UP:KEY_ACTION_DOWN, 0);
  }
}

void cec_init() {
  libcecc_reset_configuration(&g_config);
  g_config.clientVersion = LIBCEC_VERSION_CURRENT;
  g_config.bActivateSource = 0;
  g_callbacks.keyPress = &on_cec_keypress;
  g_config.callbacks = &g_callbacks;
  snprintf(g_config.strDeviceName, sizeof(g_config.strDeviceName), "Moonlight");
  g_config.callbacks = &g_callbacks;
  g_config.deviceTypes.types[0] = CEC_DEVICE_TYPE_PLAYBACK_DEVICE;
  
  if (libcecc_initialise(&g_config, &g_iface, NULL) != 1) {
    fprintf(stderr, "Failed to initialize libcec interface\n");
    fflush(stderr);
    return;
  }
  
  g_iface.init_video_standalone(g_iface.connection);
  
  cec_adapter devices[10];
  int8_t iDevicesFound = g_iface.find_adapters(g_iface.connection, devices, sizeof(devices) / sizeof(devices), NULL);
  
  if (iDevicesFound <= 0) {
    fprintf(stderr, "No CEC devices found\n");
    fflush(stderr);
    libcecc_destroy(&g_iface);
    return;
  }
  
  strcpy(g_strPort, devices[0].comm);
  if (!g_iface.open(g_iface.connection, g_strPort, 5000)) {
    fprintf(stderr, "Unable to open the device on port %s\n", g_strPort);
    fflush(stderr);
    libcecc_destroy(&g_iface);
    return;
  }
  
  g_iface.set_active_source(g_iface.connection, g_config.deviceTypes.types[0]);
}
#endif /* HAVE_LIBCEC */
