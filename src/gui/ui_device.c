#include "ui_settings.h"

#include "guilib.h"
#include "ime.h"

#include "../config.h"
#include "../device.h"
#include "../connection.h"
//#include "../debug.h"
#include "../input/vita.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>

#include <psp2/ctrl.h>
#include <psp2/rtc.h>
#include <psp2/touch.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/processmgr.h>
#include <vita2d.h>
#include <Limelight.h>
#include <client.h>
#include <errors.h>
#include <mdns.h>

#define MILLISECOND 1000
#define SECOND      (1000 * MILLISECOND)

enum {
  DEVICE_EXIT_SEARCH = 100,
  DEVICE_ITEM,
};

enum {
  DEVICE_VIEW_EXIT_SEARCH,
  DEVICE_VIEW_ITEM,
};

enum {
  SEARCH_THREAD_IDLE,
  SEARCH_THREAD_RUNNING,
  SEARCH_THREAD_REQ_STOP,
};

int search_thread_status = SEARCH_THREAD_IDLE;

static device_info_t devices[64];
static int DEVICE_ENTRY_IDX[64];
static int found_device = 0;

static char addrbuffer[64];
static char namebuffer[256];

#define NI_MAXHOST 1025
#define NI_MAXSERV 32
#define NI_NUMERICSERV 2
#define NI_NUMERICHOST 1

void ipv4_address_to_string(const struct sockaddr_in *addr, char *ip, const size_t len) {
  sceNetInetNtop(AF_INET, &addr->sin_addr.s_addr, ip, len);
}

// part of publib. BSD license
// https://github.com/ajkaijanaho/publib/blob/master/strutil/strrstr.c
char* strrstr(const char *str, const char *pat) {
  size_t len, patlen;
  const char *p;

  assert(str != NULL);
  assert(pat != NULL);

  len = strlen(str);
  patlen = strlen(pat);

  if (patlen > len)
    return NULL;
  for (p = str + (len - patlen); p > str; --p)
    if (*p == *pat && strncmp(p, pat, patlen) == 0)
      return (char *) p;
  return NULL;
}

static int mdns_discovery_callback(const struct sockaddr* from,
                                   mdns_entry_type_t entry, uint16_t type,
                                   uint16_t rclass, uint32_t ttl,
                                   const void* data, size_t size, size_t offset,
                                   size_t length) {
  switch (type) {
    case MDNS_RECORDTYPE_PTR:
      memset(&devices[found_device], 0, sizeof(device_info_t));
      break;
    case MDNS_RECORDTYPE_SRV:
      {
        mdns_record_srv_t srv = mdns_record_parse_srv(data, size, offset, length,
                                                      namebuffer, sizeof(namebuffer));
        // remove ".local." postfix
        // last . occurs problem between libgamestream and common logic
        char *p = strrstr(srv.name.str, ".local.");
        size_t len = p - srv.name.str;
        if (len > 255) {
          len = 255;
        }
        // FIXME: remove special codes
        strncpy(devices[found_device].name, srv.name.str, p - srv.name.str);
      }
      break;
    case MDNS_RECORDTYPE_A:
      {
        struct sockaddr_in addr;
        mdns_record_parse_a(data, size, offset, length, &addr);
        ipv4_address_to_string(&addr, devices[found_device].internal, 16);
      }
      break;
  }
  return 0;
}


int mdns_discovery_main(SceSize args, void *argp) {
  // conflict
  if (search_thread_status != SEARCH_THREAD_IDLE) {
    return 0;
  }

  size_t capacity = 2048;
  void* buffer = malloc(capacity);
  size_t records;

  int sock = mdns_socket_open_ipv4();
  if (sock < 0) {
    return -1;
  }
  search_thread_status = SEARCH_THREAD_RUNNING;

  if (mdns_query_send(sock, MDNS_RECORDTYPE_PTR,
                      MDNS_STRING_CONST("_nvstream._tcp.local"),
                      buffer, capacity)) {
    goto exit;
  }

  int empty_cnt = 0;
  while (search_thread_status == SEARCH_THREAD_RUNNING) {
    records = mdns_query_recv(sock, buffer, capacity, mdns_discovery_callback);
    if (records == 0) {
      empty_cnt++;
    } else {
      found_device += 1;
      empty_cnt = 0;
    }
    // wait 30sec after last receive
    if (empty_cnt >= 30 || found_device >= 50) {
      break;
    }
    sceKernelDelayThread(SECOND);
  }
exit:
  if (buffer) {
    free(buffer);
  }
  mdns_socket_close(sock);
  search_thread_status = SEARCH_THREAD_IDLE;
  return 0;
}

SceUID start_search_thread() {
  found_device = 0;
  SceUID thid = sceKernelCreateThread("mdns", mdns_discovery_main, 0x10000100, 0x10000, 0, 0, NULL);
  if (thid < 0) {
    return -1;
  }
  sceKernelStartThread(thid, 0, 0);
  return thid;
}

int end_search_thread(SceUID thid) {
  search_thread_status = SEARCH_THREAD_REQ_STOP;

  SceUInt timeout;
  do {
    timeout = 100 * MILLISECOND;
  } while (sceKernelWaitThreadEnd(thid, NULL, &timeout) < 0);

  sceKernelDeleteThread(thid);
}

static int ui_search_device_callback(int id, void *context, const input_data *input) {
  if ((input->buttons & SCE_CTRL_CROSS) == 0 || (input->buttons & SCE_CTRL_HOLD) != 0) {
    // if remain slot, reload discovered devices
    if (!DEVICE_ENTRY_IDX[DEVICE_VIEW_ITEM + found_device - 1]) {
      return 2;
    }
    return 0;
  }
  menu_entry *menu = context;
  if (id == DEVICE_EXIT_SEARCH) {
    return 1;
  }
  if (id >= DEVICE_ITEM) {
    SERVER_DATA server;

    device_info_t *info = &devices[id - DEVICE_ITEM];
    flash_message("Test connecting to:\n %s...", info->internal);
    char key_dir[4096];
    sprintf(key_dir, "%s/%s", config.key_dir, info->name);
    sceIoMkdir(key_dir, 0777);

    int ret = gs_init(&server, info->internal, key_dir, 0, true);

    if (ret == GS_OUT_OF_MEMORY) {
      display_error("Not enough memory");
      return 0;
    } else if (ret == GS_INVALID) {
      display_error("Invalid data received from server: %s\n", info->internal, gs_error);
      return 0;
    } else if (ret == GS_UNSUPPORTED_VERSION) {
      if (!config.unsupported_version) {
        display_error("Unsupported version: %s\n", gs_error);
        return 0;
      }
    } else if (ret == GS_ERROR) {
      display_error("Gamestream error: %s\n", gs_error);
      return 0;
    } else if (ret != GS_OK) {
      display_error("Can't connect to server\n%s", info->internal);
      return 0;
    }

    connection_reset();

    device_info_t *p = append_device(info);
    if (p == NULL) {
      display_error("Can't add device list\n%s", info->name);
      return 0;
    }
    info = p;

    save_device_info(info);

    if (server.paired) {
      // no more need, next action
      connection_paired();
      goto paired;
    }

    char pin[5];
    char message[256];
    sprintf(pin, "%d%d%d%d",
            (int)rand() % 10, (int)rand() % 10, (int)rand() % 10, (int)rand() % 10);
    flash_message("Please enter the following PIN\non the target PC:\n\n%s", pin);

    ret = gs_pair(&server, pin);
    if (ret != GS_OK) {
      display_error("Pairing failed: %d", ret);
      connection_terminate();
      return 0;
    }
    connection_paired();

    // if connect, need to find external ip using stun server
paired:
    info->paired = true;

    uint32_t extern_addr = 0;
    if (LiFindExternalAddressIP4("stun.stunprotocol.org", 3478, &extern_addr) == 0) {
      struct sockaddr_in addr;
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = extern_addr;
      ipv4_address_to_string(&addr, info->external, 16);
    }
    save_device_info(info);

    if (connection_terminate()) {
      display_error("Reconnect failed: %d", -2);
      return 0;
    }

    return 1;
  }
}

static int ui_search_device_back(void *context) {
  return 0;
}

int ui_search_device_loop() {
  int idx = 0;
  menu_entry menu[64];
  memset(DEVICE_ENTRY_IDX, 0, sizeof(DEVICE_ENTRY_IDX));

#define MENU_CATEGORY(NAME) \
  do { \
    menu[idx] = (menu_entry) { .name = (NAME), .disabled = true, .separator = true }; \
    idx++; \
  } while (0)
#define MENU_ENTRY(ID, TAG, NAME, SUFFIX) \
  do { \
    menu[idx] = (menu_entry) { .name = (NAME), .id = (ID), .suffix = (SUFFIX) }; \
    DEVICE_ENTRY_IDX[(TAG)] = idx; \
    idx++; \
  } while(0)
#define MENU_MESSAGE(MESSAGE) \
  do { \
    menu[idx] = (menu_entry) { .name = "", .disabled = true, .subname = (MESSAGE) }; \
    idx++; \
  } while(0)
#define MENU_SEPARATOR() \
  do { \
    menu[idx] = (menu_entry) { .name = "", .disabled = true, .separator = true }; \
    idx++; \
  } while(0)

  // TODO: sprintf
  MENU_CATEGORY("Search device ...");
  for (int i = 0; i < found_device; i++) {
    if (devices[i].internal[0] == '\0') {
      continue;
    }
    device_info_t *p = find_device(devices[i].name);
    if (p && p->paired) {
      continue;
    }
    MENU_ENTRY(DEVICE_ITEM + i, DEVICE_VIEW_ITEM + i, devices[i].name, devices[i].internal);
  }
  MENU_SEPARATOR();
  MENU_ENTRY(DEVICE_EXIT_SEARCH, DEVICE_VIEW_EXIT_SEARCH, "Return", "");

  return display_menu(menu, idx, NULL, &ui_search_device_callback, &ui_search_device_back, NULL, &menu);
}

void ui_search_device() {
  SceUID thid = start_search_thread();

  while (ui_search_device_loop() == 2);

  end_search_thread(thid);
}
