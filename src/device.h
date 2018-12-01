#pragma once
#include <stdbool.h>

typedef struct device_info device_info_t;
struct device_info {
  char name[256];
  bool paired;
  char internal[256];
  char external[256];
};

typedef struct device_infos device_infos_t;
struct device_infos {
  int size;
  int count;
  device_info_t *devices;
};

extern device_infos_t known_devices;

device_info_t* find_device(const char *name);
device_info_t* append_device();
bool update_device();
void load_all_known_devices();
bool load_device_info(device_info_t *info);
void save_device_info(const device_info_t *info);
