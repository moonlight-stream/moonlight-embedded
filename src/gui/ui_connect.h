#include <stdbool.h>
#include "../device.h"

void ui_connect_address(char *addr);
bool ui_connect_connected();

void ui_connect_saved();
void ui_connect_manual();
void ui_connect_paired_device(device_info_t *info);
