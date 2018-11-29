#include <stdlib.h>
#include <string.h>
#include <ini.h>
#include "device.h"

#define DATA_DIR "ux0:data/moonlight"
#define DEVICE_FILE "device.ini"
bool is_paired(device_info_t *info) {
  for (int i = 0; i < paired_devices.count; i++) {
    if (!strcmp(info->name, paired_devices.devices[i]->name) &&
        !strcmp(info->internal, paired_devices.devices[i]->internal)) {
      return true;
    }
  }
  return false;
}

static void device_file_path(char *out, const char *dir) {
  sprintf(out, DATA_DIR "/%s/" DEVICE_FILE, dir);
}

static int device_ini_handle(void *out, const char *section, const char *name,
                             const char *value) {
  device_info_t *info = out;
  if (strcmp(name, "internal") == 0) {
    strncpy(info->internal, value, 255);
  } else if (strcmp(name, "external") == 0) {
    strncpy(info->external, value, 255);
  }
}

void load_device_info(device_info_t *info) {
  char path[512];
  device_file_path(path, info->name);

  ini_parse(path, device_ini_handle, info);
}

void save_device_info(const device_info_t *info) {
  char path[512];
  device_file_path(path, info->name);

  FILE* fd = fopen(path, "w");

#define write_string(fd, key, value) fprintf(fd, "%s = %s\n", key, value)
  write_string(fd, "internal", info->internal);
  write_string(fd, "external", info->external);
}
