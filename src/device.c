#include "device.h"

bool is_paired(device_info_t *info) {
  for (int i = 0; i < paired_devices.count; i++) {
    if (!strcmp(info->name, paired_devices.devices[i]->name) &&
        !strcmp(info->internal, paired_devices.devices[i]->internal)) {
      return true;
    }
  }
  return false;
}

void load_device_info(const char *dir, device_info_t *info) {
}
void save_device_info(const char *dir, const device_info_t *info) {
}
