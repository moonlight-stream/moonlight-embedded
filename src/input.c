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

#include "keyboard.h"
#include "mapping.h"
#include "global.h"

#include "libevdev/libevdev.h"
#include "limelight-common/Limelight.h"

#ifdef HAVE_LIBCEC
#include <ceccloader.h>
#endif

#include <libudev.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signalfd.h>
#include <fcntl.h>
#include <poll.h>
#include <limits.h>
#include <unistd.h>
#include <pthread.h>

struct input_abs_parms {
  int min, max;
  int flat;
  int avg;
  int range, diff;
};

struct input_device {
  struct libevdev *dev;
  struct mapping map;
  int fd;
  int fdindex;
  char modifiers;
  __s32 mouseDeltaX, mouseDeltaY, mouseScroll;
  short controllerId;
  int buttonFlags;
  char leftTrigger, rightTrigger;
  short leftStickX, leftStickY;
  short rightStickX, rightStickY;
  bool gamepadModified;
  struct input_abs_parms xParms, yParms, rxParms, ryParms, zParms, rzParms;
  struct input_abs_parms dpadxParms, dpadyParms;
};

static struct pollfd* fds = NULL;
static struct input_device* devices = NULL;
static int numDevices = 0, numFds = 0;
static int assignedControllerIds = 0;

static short* currentKey;
static short* currentAbs;
static bool* currentReverse;

static bool autoadd;
static char* defaultMapfile;
static struct udev *udev;
static struct udev_monitor *udev_mon;
static int udev_fdindex;

static int sig_fdindex;

#ifdef HAVE_LIBCEC
static libcec_configuration g_config;
static char                 g_strPort[50] = { 0 };
static libcec_interface_t   g_iface;
static ICECCallbacks        g_callbacks;

static int on_cec_keypress(void* userdata, const cec_keypress key) {
  char value;
  switch (key.keycode) {
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
    short code = 0x80 << 8 | keyCodes[value];
    LiSendKeyboardEvent(code, (key.duration > 0)?KEY_ACTION_UP:KEY_ACTION_DOWN, 0);
  }
}

static void init_cec() {
  libcecc_reset_configuration(&g_config);
  g_config.clientVersion = LIBCEC_VERSION_CURRENT;
  g_config.bActivateSource = 0;
  g_callbacks.CBCecKeyPress = &on_cec_keypress;
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
#endif

static void input_init_parms(struct input_device *dev, struct input_abs_parms *parms, int code) {
  parms->flat = libevdev_get_abs_flat(dev->dev, code);
  parms->min = libevdev_get_abs_minimum(dev->dev, code);
  parms->max = libevdev_get_abs_maximum(dev->dev, code);
  parms->avg = (parms->min+parms->max)/2;
  parms->range = parms->max - parms->avg;
  parms->diff = parms->max - parms->min;
}

void input_create(const char* device, char* mapFile) {
  int fd = open(device, O_RDONLY|O_NONBLOCK);
  if (fd <= 0) {
    fprintf(stderr, "Failed to open device %s\n", device);
    fflush(stderr);
    return;
  }

  int dev = numDevices;
  int fdindex = numFds;
  numDevices++;
  numFds++;

  if (fds == NULL) {
    fds = malloc(sizeof(struct pollfd));
    devices = malloc(sizeof(struct input_device));
  } else {
    fds = realloc(fds, sizeof(struct pollfd)*numFds);
    devices = realloc(devices, sizeof(struct input_device)*numDevices);
  }

  if (fds == NULL || devices == NULL) {
    fprintf(stderr, "Not enough memory\n");
    exit(EXIT_FAILURE);
  }

  memset(&devices[dev], 0, sizeof(devices[0]));
  devices[dev].fd = fd;
  devices[dev].dev = libevdev_new();
  libevdev_set_fd(devices[dev].dev, devices[dev].fd);
  devices[dev].fdindex = fdindex;
  fds[fdindex].fd = devices[dev].fd;
  fds[fdindex].events = POLLIN;

  if (mapFile != NULL)
    mapping_load(mapFile, &(devices[dev].map));

  devices[dev].controllerId = -1;
  input_init_parms(&devices[dev], &(devices[dev].xParms), devices[dev].map.abs_x);
  input_init_parms(&devices[dev], &(devices[dev].yParms), devices[dev].map.abs_y);
  input_init_parms(&devices[dev], &(devices[dev].zParms), devices[dev].map.abs_z);
  input_init_parms(&devices[dev], &(devices[dev].rxParms), devices[dev].map.abs_rx);
  input_init_parms(&devices[dev], &(devices[dev].ryParms), devices[dev].map.abs_ry);
  input_init_parms(&devices[dev], &(devices[dev].rzParms), devices[dev].map.abs_rz);
  input_init_parms(&devices[dev], &(devices[dev].dpadxParms), devices[dev].map.abs_dpad_x);
  input_init_parms(&devices[dev], &(devices[dev].dpadyParms), devices[dev].map.abs_dpad_y);
}

static void input_remove(int devindex) {
  numDevices--;
  numFds--;

  if (devices[devindex].controllerId >= 0)
    assignedControllerIds &= ~(1 << devices[devindex].controllerId);

  int fdindex = devices[devindex].fdindex;
  if (fdindex != numFds && numFds > 0) {
    memcpy(&fds[fdindex], &fds[numFds], sizeof(struct pollfd));
    if (numFds == udev_fdindex)
      udev_fdindex = fdindex;
    else if (numFds == sig_fdindex)
      sig_fdindex = fdindex;
    else {
      for (int i=0;i<numDevices;i++) {
        if (devices[i].fdindex == numFds) {
          devices[i].fdindex = fdindex;
          break;
        }
      }
    }
  }
  if (devindex != numDevices && numDevices > 0)
    memcpy(&devices[devindex], &devices[numDevices], sizeof(struct input_device));

  fprintf(stderr, "Removed input device\n");
}

void input_init(char* mapfile) {
  #ifdef HAVE_LIBCEC
  init_cec();
  #endif

  udev = udev_new();
  if (!udev) {
    fprintf(stderr, "Can't create udev\n");
    exit(1);
  }

  autoadd = (numDevices == 0);
  if (autoadd) {
    struct udev_enumerate *enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "input");
    udev_enumerate_scan_devices(enumerate);
    struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);

    struct udev_list_entry *dev_list_entry;
    udev_list_entry_foreach(dev_list_entry, devices) {
      const char *path = udev_list_entry_get_name(dev_list_entry);
      struct udev_device *dev = udev_device_new_from_syspath(udev, path);
      const char *devnode = udev_device_get_devnode(dev);
      int id;
      if (devnode != NULL && sscanf(devnode, "/dev/input/event%d", &id) == 1) {
        input_create(devnode, mapfile);
      }
      udev_device_unref(dev);
    }

    udev_enumerate_unref(enumerate);
  }

  udev_mon = udev_monitor_new_from_netlink(udev, "udev");
  udev_monitor_filter_add_match_subsystem_devtype(udev_mon, "input", NULL);
  udev_monitor_enable_receiving(udev_mon);

  udev_fdindex = numFds++;
  sig_fdindex = numFds++;

  if (fds == NULL)
    fds = malloc(sizeof(struct pollfd)*numFds);
  else
    fds = realloc(fds, sizeof(struct pollfd)*numFds);

  if (fds == NULL) {
    fprintf(stderr, "Not enough memory\n");
    exit(EXIT_FAILURE);
  }

  defaultMapfile = mapfile;
  fds[udev_fdindex].fd = udev_monitor_get_fd(udev_mon);
  fds[udev_fdindex].events = POLLIN;

  main_thread_id = pthread_self();
  sigset_t sigset;
  sigemptyset(&sigset);
  sigaddset(&sigset, SIGHUP);
  sigaddset(&sigset, SIGTERM);
  sigaddset(&sigset, SIGINT);
  sigaddset(&sigset, SIGQUIT);
  sigprocmask(SIG_BLOCK, &sigset, NULL);
  fds[sig_fdindex].fd = signalfd(-1, &sigset, 0);
  fds[sig_fdindex].events = POLLIN | POLLERR | POLLHUP;
}

void input_destroy() {
  udev_unref(udev);
}

static short input_convert_value(struct input_event *ev, struct input_device *dev, struct input_abs_parms *parms, bool reverse) {
  if (abs(ev->value - parms->avg) < parms->flat)
    return 0;
  else if (ev->value > parms->max)
    return reverse?SHRT_MIN:SHRT_MAX;
  else if (ev->value < parms->min)
    return reverse?SHRT_MAX:SHRT_MIN;
  else if (reverse)
    return (parms->max - (ev->value<parms->avg?parms->flat*2:0) - ev->value) * (SHRT_MAX-SHRT_MIN) / (parms->max-parms->min-parms->flat*2) - SHRT_MIN;
  else
    return (ev->value - (ev->value>parms->avg?parms->flat*2:0) - parms->min) * (SHRT_MAX-SHRT_MIN) / (parms->max-parms->min-parms->flat*2) - SHRT_MIN;
}

static char input_convert_value_byte(struct input_event *ev, struct input_device *dev, struct input_abs_parms *parms) {
  if (abs(ev->value-parms->min)<parms->flat)
    return 0;
  else if (ev->value>parms->max)
    return UCHAR_MAX;
  else {
    int value = ev->value - parms->flat;
    return (value-parms->min) * UCHAR_MAX / (parms->diff-parms->flat);
  }
}

static int input_convert_value_direction(struct input_event *ev, struct input_device *dev, struct input_abs_parms *parms, bool reverse) {
  if (ev->value > (parms->avg+parms->range/4))
    return reverse?-1:1;
  else if (ev->value < (parms->avg-parms->range/4))
    return reverse?1:-1;
  else
    return 0;
}

static bool input_handle_event(struct input_event *ev, struct input_device *dev) {
  bool gamepadModified = false;

  switch (ev->type) {
  case EV_SYN:
    if (dev->mouseDeltaX != 0 || dev->mouseDeltaY != 0) {
      LiSendMouseMoveEvent(dev->mouseDeltaX, dev->mouseDeltaY);
      dev->mouseDeltaX = 0;
      dev->mouseDeltaY = 0;
    }
    if (dev->mouseScroll != 0) {
      LiSendScrollEvent(dev->mouseScroll);
      dev->mouseScroll = 0;
    }
    if (dev->gamepadModified) {
      if (dev->controllerId < 0) {
        for (int i = 0; i < 4; i++) {
          if ((assignedControllerIds & (1 << i)) == 0) {
            assignedControllerIds |= (1 << i);
            dev->controllerId = i;
            break;
          }
        }
        //Use id 0 when too many gamepads are connected
        if (dev->controllerId < 0)
          dev->controllerId = 0;
      }
      LiSendMultiControllerEvent(dev->controllerId, dev->buttonFlags, dev->leftTrigger, dev->rightTrigger, dev->leftStickX, dev->leftStickY, dev->rightStickX, dev->rightStickY);
      dev->gamepadModified = false;
    }
    break;
  case EV_KEY:
    if (ev->code < sizeof(keyCodes)/sizeof(keyCodes[0])) {
      char modifier = 0;
      switch (ev->code) {
      case KEY_LEFTSHIFT:
      case KEY_RIGHTSHIFT:
        modifier = MODIFIER_SHIFT;
        break;
      case KEY_LEFTALT:
      case KEY_RIGHTALT:
        modifier = MODIFIER_ALT;
        break;
      case KEY_LEFTCTRL:
      case KEY_RIGHTCTRL:
        modifier = MODIFIER_CTRL;
        break;
      }
      if (modifier != 0) {
        if (ev->value)
          dev->modifiers |= modifier;
        else
          dev->modifiers &= ~modifier;
      }

      short code = 0x80 << 8 | keyCodes[ev->code];
      LiSendKeyboardEvent(code, ev->value?KEY_ACTION_DOWN:KEY_ACTION_UP, dev->modifiers);
    } else {
      int mouseCode = 0;
      short gamepadCode = 0;
      switch (ev->code) {
      case BTN_LEFT:
        mouseCode = BUTTON_LEFT;
        break;
      case BTN_MIDDLE:
        mouseCode = BUTTON_MIDDLE;
        break;
      case BTN_RIGHT:
        mouseCode = BUTTON_RIGHT;
        break;
      default:
        if (ev->code == dev->map.btn_south)
          gamepadCode = A_FLAG;
        else if (ev->code == dev->map.btn_west)
          gamepadCode = X_FLAG;
        else if (ev->code == dev->map.btn_north)
          gamepadCode = Y_FLAG;
        else if (ev->code == dev->map.btn_east)
          gamepadCode = B_FLAG;
        else if (ev->code == dev->map.btn_dpad_up)
          gamepadCode = UP_FLAG;
        else if (ev->code == dev->map.btn_dpad_down)
          gamepadCode = DOWN_FLAG;
        else if (ev->code == dev->map.btn_dpad_right)
          gamepadCode = RIGHT_FLAG;
        else if (ev->code == dev->map.btn_dpad_left)
          gamepadCode = LEFT_FLAG;
        else if (ev->code == dev->map.btn_thumbl)
          gamepadCode = LS_CLK_FLAG;
        else if (ev->code == dev->map.btn_thumbr)
          gamepadCode = RS_CLK_FLAG;
        else if (ev->code == dev->map.btn_tl)
          gamepadCode = LB_FLAG;
        else if (ev->code == dev->map.btn_tr)
          gamepadCode = RB_FLAG;
        else if (ev->code == dev->map.btn_start)
          gamepadCode = PLAY_FLAG;
        else if (ev->code == dev->map.btn_select)
          gamepadCode = BACK_FLAG;
        else if (ev->code == dev->map.btn_mode)
          gamepadCode = SPECIAL_FLAG;
      }

      if (mouseCode != 0) {
        LiSendMouseButtonEvent(ev->value?BUTTON_ACTION_PRESS:BUTTON_ACTION_RELEASE, mouseCode);
      } else {
        gamepadModified = true;

        if (gamepadCode != 0) {
          if (ev->value)
            dev->buttonFlags |= gamepadCode;
          else
            dev->buttonFlags &= ~gamepadCode;
        } else if (ev->code == dev->map.btn_tl2)
          dev->leftTrigger = ev->value?UCHAR_MAX:0;
        else if (ev->code == dev->map.btn_tr2)
          dev->rightTrigger = ev->value?UCHAR_MAX:0;
        else {
          fprintf(stderr, "Unmapped button: %d\n", ev->code);
          gamepadModified = false;
        }
      }
    }
    break;
  case EV_REL:
    switch (ev->code) {
      case REL_X:
        dev->mouseDeltaX = ev->value;
        break;
      case REL_Y:
        dev->mouseDeltaY = ev->value;
        break;
      case REL_WHEEL:
        dev->mouseScroll = ev->value;
        break;
    }
    break;
  case EV_ABS:
    gamepadModified = true;
    if (ev->code == dev->map.abs_x)
      dev->leftStickX = input_convert_value(ev, dev, &dev->xParms, dev->map.reverse_x);
    else if (ev->code == dev->map.abs_y)
      dev->leftStickY = input_convert_value(ev, dev, &dev->yParms, dev->map.reverse_y);
    else if (ev->code == dev->map.abs_rx)
      dev->rightStickX = input_convert_value(ev, dev, &dev->rxParms, dev->map.reverse_rx);
    else if (ev->code == dev->map.abs_ry)
      dev->rightStickY = input_convert_value(ev, dev, &dev->ryParms, dev->map.reverse_ry);
    else if (ev->code == dev->map.abs_z)
      dev->leftTrigger = input_convert_value_byte(ev, dev, &dev->zParms);
    else if (ev->code == dev->map.abs_rz)
      dev->rightTrigger = input_convert_value_byte(ev, dev, &dev->rzParms);
    else if (ev->code == dev->map.abs_dpad_x) {
      int dir = input_convert_value_direction(ev, dev, &dev->dpadxParms, dev->map.reverse_dpad_x);
      if (dir == 1) {
        dev->buttonFlags |= RIGHT_FLAG;
        dev->buttonFlags &= ~LEFT_FLAG;
      } else if (dir == 0) {
        dev->buttonFlags &= ~RIGHT_FLAG;
        dev->buttonFlags &= ~LEFT_FLAG;
      } else {
        dev->buttonFlags &= ~RIGHT_FLAG;
        dev->buttonFlags |= LEFT_FLAG;
      }
    } else if (ev->code == dev->map.abs_dpad_y) {
      int dir = input_convert_value_direction(ev, dev, &dev->dpadyParms, dev->map.reverse_dpad_y);
      if (dir == 1) {
        dev->buttonFlags |= DOWN_FLAG;
        dev->buttonFlags &= ~UP_FLAG;
      } else if (dir == 0) {
        dev->buttonFlags &= ~DOWN_FLAG;
        dev->buttonFlags &= ~UP_FLAG;
      } else {
        dev->buttonFlags &= ~DOWN_FLAG;
        dev->buttonFlags |= UP_FLAG;
      }
    } else
      gamepadModified = false;

    break;
  }

  dev->gamepadModified |= gamepadModified;
  return true;
}

static bool input_handle_mapping_event(struct input_event *ev, struct input_device *dev) {
  switch (ev->type) {
  case EV_KEY:
    if (currentKey != NULL) {
      if (ev->value)
        *currentKey = ev->code;
      else if (ev->code == *currentKey)
        return false;
    }
    break;
  case EV_ABS:
    if (currentAbs != NULL) {
      struct input_abs_parms parms;
      input_init_parms(dev, &parms, ev->code);

      if (ev->value > parms.avg + parms.range/2) {
        *currentAbs = ev->code;
        *currentReverse = false;
      } else if (ev->value < parms.avg - parms.range/2) {
        *currentAbs = ev->code;
        *currentReverse = true;
      } else if (ev->code == *currentAbs)
        return false;
    }
    break;
  }
  return true;
}

static void input_drain(void) {
  for (int i = 0; i < numDevices; i++) {
    struct input_event ev;
    while (libevdev_next_event(devices[i].dev, LIBEVDEV_READ_FLAG_NORMAL, &ev) >= 0);
  }
}

static bool input_poll(bool (*handler) (struct input_event*, struct input_device*)) {
  while (poll(fds, numFds, -1)) {
    if (fds[udev_fdindex].revents > 0) {
      struct udev_device *dev = udev_monitor_receive_device(udev_mon);
      const char *action = udev_device_get_action(dev);
      if (action != NULL) {
        if (autoadd && strcmp("add", action) == 0) {
          const char *devnode = udev_device_get_devnode(dev);
          int id;
          if (devnode != NULL && sscanf(devnode, "/dev/input/event%d", &id) == 1) {
            input_create(devnode, defaultMapfile);
          }
        }
        udev_device_unref(dev);
      }
    } else if (fds[sig_fdindex].revents > 0) {
      struct signalfd_siginfo info;
      read(fds[sig_fdindex].fd, &info, sizeof(info));

      switch (info.ssi_signo) {
        case SIGINT:
        case SIGTERM:
        case SIGQUIT:
        case SIGHUP:
          return false;
      }
    }
    for (int i=0;i<numDevices;i++) {
      if (fds[devices[i].fdindex].revents > 0) {
        int rc;
        struct input_event ev;
        while ((rc = libevdev_next_event(devices[i].dev, LIBEVDEV_READ_FLAG_NORMAL, &ev)) >= 0) {
          if (rc == LIBEVDEV_READ_STATUS_SYNC)
            fprintf(stderr, "Error: cannot keep up\n");
          else if (rc == LIBEVDEV_READ_STATUS_SUCCESS) {
            if (!handler(&ev, &devices[i]))
              return true;
          }
        }
        if (rc == -ENODEV) {
          input_remove(i);
        } else if (rc != -EAGAIN && rc < 0) {
          fprintf(stderr, "Error: %s\n", strerror(-rc));
          exit(EXIT_FAILURE);
        }
      }
    }
  }

  return false;
}

static void input_map_key(char* keyName, short* key) {
  printf("Press %s\n", keyName);
  currentKey = key;
  currentAbs = NULL;
  *key = -1;
  if (!input_poll(input_handle_mapping_event))
    exit(1);

  usleep(250000);
  input_drain();
}

static void input_map_abs(char* keyName, short* abs, bool* reverse) {
  printf("Move %s\n", keyName);
  currentKey = NULL;
  currentAbs = abs;
  currentReverse = reverse;
  *abs = -1;
  if (!input_poll(input_handle_mapping_event))
    exit(1);

  usleep(250000);
  input_drain();
}

static void input_map_abskey(char* keyName, short* key, short* abs, bool* reverse) {
  printf("Press %s\n", keyName);
  currentKey = key;
  currentAbs = abs;
  currentReverse = reverse;
  *key = -1;
  *abs = -1;
  *currentReverse = false;
  if (!input_poll(input_handle_mapping_event))
    exit(1);

  usleep(250000);
  input_drain();
}

void input_map(char* fileName) {
  struct mapping map;

  input_map_abs("Left Stick Right", &(map.abs_x), &(map.reverse_x));
  input_map_abs("Left Stick Up", &(map.abs_y), &(map.reverse_y));
  input_map_key("Left Stick Button", &(map.btn_thumbl));

  input_map_abs("Right Stick Right", &(map.abs_rx), &(map.reverse_rx));
  input_map_abs("Right Stick Up", &(map.abs_ry), &(map.reverse_ry));
  input_map_key("Right Stick Button", &(map.btn_thumbr));

  input_map_abskey("D-Pad Right", &(map.btn_dpad_right), &(map.abs_dpad_x), &(map.reverse_dpad_x));
  if (map.btn_dpad_right >= 0)
    input_map_key("D-Pad Left", &(map.btn_dpad_left));
  else
    map.btn_dpad_left = -1;

  input_map_abskey("D-Pad Down", &(map.btn_dpad_down), &(map.abs_dpad_y), &(map.reverse_dpad_y));
  if (map.btn_dpad_down >= 0)
    input_map_key("D-Pad Up", &(map.btn_dpad_up));
  else
    map.btn_dpad_up = -1;

  input_map_key("Button X (1)", &(map.btn_west));
  input_map_key("Button A (2)", &(map.btn_south));
  input_map_key("Button B (3)", &(map.btn_east));
  input_map_key("Button Y (4)", &(map.btn_north));
  input_map_key("Back Button", &(map.btn_select));
  input_map_key("Start Button", &(map.btn_start));
  input_map_key("Special Button", &(map.btn_mode));

  bool ignored;
  input_map_abskey("Left Trigger", &(map.btn_tl2), &(map.abs_z), &ignored);
  input_map_abskey("Right Trigger", &(map.btn_tr2), &(map.abs_rz), &ignored);

  input_map_key("Left Bumper", &(map.btn_tl));
  input_map_key("Right Bumper", &(map.btn_tr));
  mapping_save(fileName, &map);
}

void input_loop() {
  input_poll(input_handle_event);
}
