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

#include "../loop.h"

#include "evdev.h"

#include <libudev.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>

static bool autoadd;
static char* defaultMapfile;

static struct udev *udev;
static struct udev_monitor *udev_mon;
static int udev_fd;

static int udev_handle(int fd) {
  struct udev_device *dev = udev_monitor_receive_device(udev_mon);
  const char *action = udev_device_get_action(dev);
  if (action != NULL) {
    if (autoadd && strcmp("add", action) == 0) {
      const char *devnode = udev_device_get_devnode(dev);
      int id;
      if (devnode != NULL && sscanf(devnode, "/dev/input/event%d", &id) == 1) {
        evdev_create(devnode, defaultMapfile);
      }
    }
    udev_device_unref(dev);
  }
  return LOOP_OK;
}

void udev_init(bool autoload, char* mapfile) {
  udev = udev_new();
  if (!udev) {
    fprintf(stderr, "Can't create udev\n");
    exit(1);
  }

  autoadd = autoload;
  if (autoload) {
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
        evdev_create(devnode, mapfile);
      }
      udev_device_unref(dev);
    }

    udev_enumerate_unref(enumerate);
  }

  udev_mon = udev_monitor_new_from_netlink(udev, "udev");
  udev_monitor_filter_add_match_subsystem_devtype(udev_mon, "input", NULL);
  udev_monitor_enable_receiving(udev_mon);

  defaultMapfile = mapfile;

  int udev_fd = udev_monitor_get_fd(udev_mon);
  loop_add_fd(udev_fd, &udev_handle, POLLIN);
}

void evdev_destroy() {
  udev_unref(udev);
}
