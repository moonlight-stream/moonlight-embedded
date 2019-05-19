#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ini.h>

#include <psp2/io/dirent.h>

#include "device.h"
#include "debug.h"

#define DATA_DIR "ux0:data/moonlight"
#define DEVICE_FILE "device.ini"

#define BOOL(v) strcmp((v), "true") == 0
#define write_bool(fd, key, value) fprintf(fd, "%s = %s\n", key, value ? "true" : "false");
#define write_string(fd, key, value) fprintf(fd, "%s = %s\n", key, value)

device_infos_t known_devices = {0};

device_info_t* find_device(const char *name) {
  // TODO: mutex
  for (int i = 0; i < known_devices.count; i++) {
    if (!strcmp(name, known_devices.devices[i].name)) {
      return &known_devices.devices[i];
    }
  }
  return NULL;
}

static void device_file_path(char *out, const char *dir) {
  snprintf(out, 512, DATA_DIR "/%s/" DEVICE_FILE, dir);
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
  return 1;
}

device_info_t* append_device(device_info_t *info) {
  if (find_device(info->name)) {
    return NULL;
  }
  // FIXME: need mutex
  if (known_devices.size == 0) {
    known_devices.devices = malloc(sizeof(device_info_t) * 4);
    if (known_devices.devices == NULL) {
      return NULL;
    }
    known_devices.size = 4;
  } else if (known_devices.size == known_devices.count) {
    //if (known_devices.size == 64) {
    //  return false;
    //}
    size_t new_size = sizeof(device_info_t) * (known_devices.size * 2);
    device_info_t *tmp = realloc(known_devices.devices, new_size);
    if (tmp == NULL) {
      return NULL;
    }
    known_devices.devices = tmp;
    known_devices.size *= 2;
  }
  device_info_t *p = &known_devices.devices[known_devices.count];

  strncpy(p->name, info->name, 255);
  p->paired = info->paired;
  strncpy(p->internal, info->internal, 255);
  strncpy(p->external, info->external, 255);

  known_devices.count++;
  return p;
}

bool update_device(device_info_t *info) {
  device_info_t *p = find_device(info->name);
  if (p == NULL) {
    return false;
  }
  //strncpy(p->name, info->name, 255);
  p->paired = info->paired;
  strncpy(p->internal, info->internal, 255);
  strncpy(p->external, info->external, 255);
  return true;
}

void load_all_known_devices() {
  struct stat st;
  device_info_t info;

  SceUID dfd = sceIoDopen(DATA_DIR);
  if (dfd < 0) {
    return;
  }
  do {
    SceIoDirent ent = {0};
    if (sceIoDread(dfd, &ent) <= 0) {
      break;
    }
    if (strcmp(".", ent.d_name) == 0 || strcmp("..", ent.d_name) == 0) {
      continue;
    }
    if (!SCE_S_ISDIR(ent.d_stat.st_mode)) {
      continue;
    }

    memset(&info, 0, sizeof(device_info_t));
    strncpy(info.name, ent.d_name, 255);
    if (!load_device_info(&info)) {
      continue;
    }
    append_device(&info);
  } while(true);

  sceIoDclose(dfd);
  return;
}

bool load_device_info(device_info_t *info) {
  char path[512] = {0};
  device_file_path(path, info->name);
  vita_debug_log("%s\n", path);

  int ret = ini_parse(path, device_ini_handle, info);
  vita_debug_log("%d\n", ret);
  return ret == 0;
}

void save_device_info(const device_info_t *info) {
  char path[512] = {0};
  device_file_path(path, info->name);

  FILE* fd = fopen(path, "w");
  if (!fd) {
    // FIXME
    return;
  }

  write_bool(fd, "paired", info->paired);
  write_string(fd, "internal", info->internal);
  write_string(fd, "external", info->external);
  fclose(fd);
}
