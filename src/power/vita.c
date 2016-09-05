/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2015 Iwan Timmer, Sunguk Lee
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
#include <stdbool.h>

#include <psp2/kernel/processmgr.h>
#include <psp2/power.h>
#include "../config.h"

enum {
  ENABLE_ALL = 0,
  DISABLE_SUSPEND = 1,
};

static int powermode = ENABLE_ALL;
static bool active_power_thread = false;

int vitapower_thread(SceSize args, void *argp) {
  while (1) {
    if (!active_power_thread) {
      sceKernelDelayThread(10 * 1000 * 1000);
      continue;
    }
    if (powermode & DISABLE_SUSPEND) {
      sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DISABLE_AUTO_SUSPEND);
      sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DISABLE_OLED_OFF);
    }
    if (!scePowerIsBatteryCharging() && scePowerIsLowBattery()) {
      // TODO print warning message
    }
    sceKernelDelayThread(10 * 1000 * 1000);
  }

  return 0;
}

bool vitapower_init() {
  SceUID thid = sceKernelCreateThread("vitapower_thread", vitapower_thread, 0x10000100, 0x40000, 0, 0, NULL);
  if (thid >= 0)
    sceKernelStartThread(thid, 0, NULL);

  return true;
}

void vitapower_config(CONFIGURATION config) {
  powermode = ENABLE_ALL;

  if (config.disable_powersave) {
    powermode |= DISABLE_SUSPEND;
  }
}

void vitapower_start() {
  active_power_thread = true;
}

void vitapower_stop() {
  active_power_thread = false;
}
