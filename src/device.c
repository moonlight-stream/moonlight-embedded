#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ini.h>
#include "device.h"

#define DATA_DIR "ux0:data/moonlight"
#define DEVICE_FILE "device.ini"

#define BOOL(v) strcmp((v), "true") == 0

bool is_paired(device_info_t *info) {
  for (int i = 0; i < known_devices.count; i++) {
    if (!strcmp(info->name, known_devices.devices[i].name) &&
        !strcmp(info->internal, known_devices.devices[i].internal)) {
      return known_devices.devices[i].paired;
    }
  }
  return false;
}

static int device_file_path(char *out, const char *dir) {
  sprintf(out, DATA_DIR "/%s/" DEVICE_FILE, dir);
  return access(out, F_OK);
}

static int device_ini_handle(void *out, const char *section, const char *name,
                             const char *value) {
  device_info_t *info = out;
  if (strcmp(name, "paired") == 0) {
    info->paired = BOOL(value);
  } else if (strcmp(name, "internal") == 0) {
    strncpy(info->internal, value, 255);
  } else if (strcmp(name, "external") == 0) {
    strncpy(info->external, value, 255);
  }
}

bool append_device(device_info_t *info) {
  if (known_devices.size == 0) {
    known_devices.devices = malloc(sizeof(device_info_t) * 4);
    if (known_devices.devices == NULL) {
      return false;
    }
    known_devices.size = 0;
  } else if (known_devices.size == known_devices.count) {
    //if (known_devices.size == 64) {
    //  return false;
    //}
    size_t new_size = sizeof(device_info_t) * (known_devices.size * 2);
    device_info_t *tmp = realloc(known_devices.devices, new_size);
    if (tmp == NULL) {
      return false;
    }
    known_devices.devices = tmp;
  }
  device_info_t *p = &known_devices.devices[known_devices.count];

  strncpy(p->name, info->name, 255);
  p->paired = info->paired;
  strncpy(p->internal, info->internal, 255);
  strncpy(p->external, info->external, 255);

  known_devices.count++;
}

void load_all_known_devices() {
  struct dirent *ent;
  char path[512];
  struct stat st;
  device_info_t info;

  DIR *dp = opendir(DATA_DIR);
  while (ent = readdir(dp)) {
    if (strcmp(".", ent->d_name) == 0 || strcmp("..", ent->d_name)) {
      continue;
    }
    sprintf(path, DATA_DIR "/%s", ent->d_name);
    if (stat(path, &st)) {
      continue;
    }
    if (!S_ISDIR(st.st_mode)) {
      continue;
    }
    if (!device_file_path(path, ent->d_name)) {
      continue;
    }

    memset(&info, 0, sizeof(device_info_t));
    strncpy(info.name, ent->d_name, 255);
    load_device_info(&info);
    append_device(&info);
  }
  closedir(dp);
  return;
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

#define write_bool(fd, key, value) fprintf(fd, "%s = %s\n", key, value ? "true" : "false");
#define write_string(fd, key, value) fprintf(fd, "%s = %s\n", key, value)
  write_bool(fd, "paired", info->paired);
  write_string(fd, "internal", info->internal);
  write_string(fd, "external", info->external);
}
