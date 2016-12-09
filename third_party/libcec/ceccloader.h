#pragma once
/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2015 Pulse-Eight Limited.  All rights reserved.
 * libCEC(R) is an original work, containing original code.
 *
 * libCEC(R) is a trademark of Pulse-Eight Limited.
 *
 * This program is dual-licensed; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * Alternatively, you can license this library under a commercial license,
 * please contact Pulse-Eight Licensing for more information.
 *
 * For more information contact:
 * Pulse-Eight Licensing       <license@pulse-eight.com>
 *     http://www.pulse-eight.com/
 *     http://www.pulse-eight.net/
 */

#include "cecc.h"

#include <stdio.h>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <conio.h>
typedef HINSTANCE libcecc_lib_instance_t;
#else
#include <dlfcn.h>
typedef void* libcecc_lib_instance_t;
#ifndef CDECL
#define CDECL
#endif
#endif

static libcecc_lib_instance_t libcecc_load_library(const char* strLib);
static void libcecc_close_library(libcecc_lib_instance_t lib);
static void* libcecc_resolve(void* lib, const char* name);

#define _libcecc_resolve(lib, tar, name, method) \
  do { \
    tar = (method) libcecc_resolve(lib, name); \
    if (tar == NULL) \
    { \
      libcecc_close_library(lib); \
      return -1; \
    } \
  } while(0)

typedef struct {
  libcec_connection_t                 connection;
  libcecc_lib_instance_t              lib_instance;
  void                                (CDECL *destroy)(libcec_connection_t connection);
  int                                 (CDECL *open)(libcec_connection_t connection, const char* strPort, uint32_t iTimeout);
  void                                (CDECL *close)(libcec_connection_t connection);
  void                                (CDECL *clear_configuration)(CEC_NAMESPACE libcec_configuration* configuration);
  int                                 (CDECL *enable_callbacks)(libcec_connection_t connection, void* cbParam, CEC_NAMESPACE ICECCallbacks* callbacks);
  int8_t                              (CDECL *find_adapters)(libcec_connection_t connection, CEC_NAMESPACE cec_adapter* deviceList, uint8_t iBufSize, const char* strDevicePath);
  int                                 (CDECL *ping_adapters)(libcec_connection_t connection);
  int                                 (CDECL *start_bootloader)(libcec_connection_t connection);
  int                                 (CDECL *power_on_devices)(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address address);
  int                                 (CDECL *standby_devices)(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address address);
  int                                 (CDECL *set_active_source)(libcec_connection_t connection, CEC_NAMESPACE cec_device_type type);
  int                                 (CDECL *set_deck_control_mode)(libcec_connection_t connection, CEC_NAMESPACE cec_deck_control_mode mode, int bSendUpdate);
  int                                 (CDECL *set_deck_info)(libcec_connection_t connection, CEC_NAMESPACE cec_deck_info info, int bSendUpdate);
  int                                 (CDECL *set_inactive_view)(libcec_connection_t connection);
  int                                 (CDECL *set_menu_state)(libcec_connection_t connection, CEC_NAMESPACE cec_menu_state state, int bSendUpdate);
  int                                 (CDECL *transmit)(libcec_connection_t connection, const CEC_NAMESPACE cec_command* data);
  int                                 (CDECL *set_logical_address)(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address iLogicalAddress);
  int                                 (CDECL *set_physical_address)(libcec_connection_t connection, uint16_t iPhysicalAddress);
  int                                 (CDECL *set_osd_string)(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address iLogicalAddress, CEC_NAMESPACE cec_display_control duration, const char* strMessage);
  int                                 (CDECL *switch_monitoring)(libcec_connection_t connection, int bEnable);
  CEC_NAMESPACE cec_version           (CDECL *get_device_cec_version)(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address iLogicalAddress);
  int                                 (CDECL *get_device_menu_language)(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address iLogicalAddress, CEC_NAMESPACE cec_menu_language language);
  uint64_t                            (CDECL *get_device_vendor_id)(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address iLogicalAddress);
  uint16_t                            (CDECL *get_device_physical_address)(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address iLogicalAddress);
  CEC_NAMESPACE cec_logical_address   (CDECL *get_active_source)(libcec_connection_t connection);
  int                                 (CDECL *is_active_source)(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address iAddress);
  CEC_NAMESPACE cec_power_status      (CDECL *get_device_power_status)(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address iLogicalAddress);
  int                                 (CDECL *poll_device)(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address iLogicalAddress);
  CEC_NAMESPACE cec_logical_addresses (CDECL *get_active_devices)(libcec_connection_t connection);
  int                                 (CDECL *is_active_device)(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address address);
  int                                 (CDECL *is_active_device_type)(libcec_connection_t connection, CEC_NAMESPACE cec_device_type type);
  int                                 (CDECL *set_hdmi_port)(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address baseDevice, uint8_t iPort);
  int                                 (CDECL *volume_up)(libcec_connection_t connection, int bSendRelease);
  int                                 (CDECL *volume_down)(libcec_connection_t connection, int bSendRelease);
  int                                 (CDECL *mute_audio)(libcec_connection_t connection, int bSendRelease);
  int                                 (CDECL *send_keypress)(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address iDestination, CEC_NAMESPACE cec_user_control_code key, int bWait);
  int                                 (CDECL *send_key_release)(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address iDestination, int bWait);
  int                                 (CDECL *get_device_osd_name)(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address iAddress, CEC_NAMESPACE cec_osd_name name);
  int                                 (CDECL *set_stream_path_logical)(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address iAddress);
  int                                 (CDECL *set_stream_path_physical)(libcec_connection_t connection, uint16_t iPhysicalAddress);
  CEC_NAMESPACE cec_logical_addresses (CDECL *get_logical_addresses)(libcec_connection_t connection);
  int                                 (CDECL *get_current_configuration)(libcec_connection_t connection, CEC_NAMESPACE libcec_configuration* configuration);
  int                                 (CDECL *can_persist_configuration)(libcec_connection_t connection);
  int                                 (CDECL *persist_configuration)(libcec_connection_t connection, CEC_NAMESPACE libcec_configuration* configuration);
  int                                 (CDECL *set_configuration)(libcec_connection_t connection, const CEC_NAMESPACE libcec_configuration* configuration);
  void                                (CDECL *rescan_devices)(libcec_connection_t connection);
  int                                 (CDECL *is_libcec_active_source)(libcec_connection_t connection);
  int                                 (CDECL *get_device_information)(libcec_connection_t connection, const char* strPort, CEC_NAMESPACE libcec_configuration* config, uint32_t iTimeoutMs);
  const char*                         (CDECL *get_lib_info)(libcec_connection_t connection);
  void                                (CDECL *init_video_standalone)(libcec_connection_t connection);
  uint16_t                            (CDECL *get_adapter_vendor_id)(libcec_connection_t connection);
  uint16_t                            (CDECL *get_adapter_product_id)(libcec_connection_t connection);
  uint8_t                             (CDECL *audio_toggle_mute)(libcec_connection_t connection);
  uint8_t                             (CDECL *audio_mute)(libcec_connection_t connection);
  uint8_t                             (CDECL *audio_unmute)(libcec_connection_t connection);
  uint8_t                             (CDECL *audio_get_status)(libcec_connection_t connection);
  int8_t                              (CDECL *detect_adapters)(libcec_connection_t connection, CEC_NAMESPACE cec_adapter_descriptor* deviceList, uint8_t iBufSize, const char* strDevicePath, int bQuickScan);
  void                                (CDECL *menu_state_to_string)(const CEC_NAMESPACE cec_menu_state state, char* buf, size_t bufsize);
  void                                (CDECL *cec_version_to_string)(const CEC_NAMESPACE cec_version version, char* buf, size_t bufsize);
  void                                (CDECL *power_status_to_string)(const CEC_NAMESPACE cec_power_status status, char* buf, size_t bufsize);
  void                                (CDECL *logical_address_to_string)(const CEC_NAMESPACE cec_logical_address address, char* buf, size_t bufsize);
  void                                (CDECL *deck_control_mode_to_string)(const CEC_NAMESPACE cec_deck_control_mode mode, char* buf, size_t bufsize);
  void                                (CDECL *deck_status_to_string)(const CEC_NAMESPACE cec_deck_info status, char* buf, size_t bufsize);
  void                                (CDECL *opcode_to_string)(const CEC_NAMESPACE cec_opcode opcode, char* buf, size_t bufsize);
  void                                (CDECL *system_audio_status_to_string)(const CEC_NAMESPACE cec_system_audio_status mode, char* buf, size_t bufsize);
  void                                (CDECL *audio_status_to_string)(const CEC_NAMESPACE cec_audio_status status, char* buf, size_t bufsize);
  void                                (CDECL *vendor_id_to_string)(const CEC_NAMESPACE cec_vendor_id vendor, char* buf, size_t bufsize);
  void                                (CDECL *user_control_key_to_string)(const CEC_NAMESPACE cec_user_control_code key, char* buf, size_t bufsize);
  void                                (CDECL *adapter_type_to_string)(const CEC_NAMESPACE cec_adapter_type type, char* buf, size_t bufsize);
  void                                (CDECL *version_to_string)(uint32_t version, char* buf, size_t bufsize);
} libcec_interface_t;

static int libcecc_resolve_all(void* lib, libcec_interface_t* iface)
{
  if (!lib || !iface)
    return -1;

  _libcecc_resolve(lib, iface->destroy,                       "libcec_destroy",                       void(CDECL *)(libcec_connection_t));
  _libcecc_resolve(lib, iface->open,                          "libcec_open",                          int(CDECL *)(libcec_connection_t, const char*, uint32_t));
  _libcecc_resolve(lib, iface->close,                         "libcec_close",                         void(CDECL *)(libcec_connection_t));
  _libcecc_resolve(lib, iface->clear_configuration,           "libcec_clear_configuration",           void(CDECL *)(CEC_NAMESPACE libcec_configuration*));
  _libcecc_resolve(lib, iface->enable_callbacks,              "libcec_enable_callbacks",              int(CDECL *)(libcec_connection_t, void*, CEC_NAMESPACE ICECCallbacks*));
  _libcecc_resolve(lib, iface->find_adapters,                 "libcec_find_adapters",                 int8_t(CDECL *)(libcec_connection_t, CEC_NAMESPACE cec_adapter*, uint8_t, const char*));
  _libcecc_resolve(lib, iface->ping_adapters,                 "libcec_ping_adapters",                 int(CDECL *)(libcec_connection_t));
  _libcecc_resolve(lib, iface->start_bootloader,              "libcec_start_bootloader",              int(CDECL *)(libcec_connection_t));
  _libcecc_resolve(lib, iface->power_on_devices,              "libcec_power_on_devices",              int(CDECL *)(libcec_connection_t, CEC_NAMESPACE cec_logical_address));
  _libcecc_resolve(lib, iface->standby_devices,               "libcec_standby_devices",               int(CDECL *)(libcec_connection_t, CEC_NAMESPACE cec_logical_address));
  _libcecc_resolve(lib, iface->set_active_source,             "libcec_set_active_source",             int(CDECL *)(libcec_connection_t, CEC_NAMESPACE cec_device_type));
  _libcecc_resolve(lib, iface->set_deck_control_mode,         "libcec_set_deck_control_mode",         int(CDECL *)(libcec_connection_t, CEC_NAMESPACE cec_deck_control_mode, int));
  _libcecc_resolve(lib, iface->set_deck_info,                 "libcec_set_deck_info",                 int(CDECL *)(libcec_connection_t, CEC_NAMESPACE cec_deck_info, int));
  _libcecc_resolve(lib, iface->set_inactive_view,             "libcec_set_inactive_view",             int(CDECL *)(libcec_connection_t));
  _libcecc_resolve(lib, iface->set_menu_state,                "libcec_set_menu_state",                int(CDECL *)(libcec_connection_t, CEC_NAMESPACE cec_menu_state, int));
  _libcecc_resolve(lib, iface->transmit,                      "libcec_transmit",                      int(CDECL *)(libcec_connection_t, const CEC_NAMESPACE cec_command*));
  _libcecc_resolve(lib, iface->set_logical_address,           "libcec_set_logical_address",           int(CDECL *)(libcec_connection_t, CEC_NAMESPACE cec_logical_address));
  _libcecc_resolve(lib, iface->set_physical_address,          "libcec_set_physical_address",          int(CDECL *)(libcec_connection_t, uint16_t));
  _libcecc_resolve(lib, iface->set_osd_string,                "libcec_set_osd_string",                int(CDECL *)(libcec_connection_t, CEC_NAMESPACE cec_logical_address, CEC_NAMESPACE cec_display_control, const char*));
  _libcecc_resolve(lib, iface->switch_monitoring,             "libcec_switch_monitoring",             int(CDECL *)(libcec_connection_t, int));
  _libcecc_resolve(lib, iface->get_device_cec_version,        "libcec_get_device_cec_version",        CEC_NAMESPACE cec_version(CDECL *)(libcec_connection_t, CEC_NAMESPACE cec_logical_address));
  _libcecc_resolve(lib, iface->get_device_menu_language,      "libcec_get_device_menu_language",      int(CDECL *)(libcec_connection_t, CEC_NAMESPACE cec_logical_address, CEC_NAMESPACE cec_menu_language));
  _libcecc_resolve(lib, iface->get_device_vendor_id,          "libcec_get_device_vendor_id",          uint64_t(CDECL *)(libcec_connection_t, CEC_NAMESPACE cec_logical_address));
  _libcecc_resolve(lib, iface->get_device_physical_address,   "libcec_get_device_physical_address",   uint16_t(CDECL *)(libcec_connection_t, CEC_NAMESPACE cec_logical_address));
  _libcecc_resolve(lib, iface->get_active_source,             "libcec_get_active_source",             CEC_NAMESPACE cec_logical_address(CDECL *)(libcec_connection_t));
  _libcecc_resolve(lib, iface->is_active_source,              "libcec_is_active_source",              int(CDECL *)(libcec_connection_t, CEC_NAMESPACE cec_logical_address));
  _libcecc_resolve(lib, iface->get_device_power_status,       "libcec_get_device_power_status",       CEC_NAMESPACE cec_power_status(CDECL *)(libcec_connection_t, CEC_NAMESPACE cec_logical_address));
  _libcecc_resolve(lib, iface->poll_device,                   "libcec_poll_device",                   int(CDECL *)(libcec_connection_t, CEC_NAMESPACE cec_logical_address));
  _libcecc_resolve(lib, iface->get_active_devices,            "libcec_get_active_devices",            CEC_NAMESPACE cec_logical_addresses(CDECL *)(libcec_connection_t));
  _libcecc_resolve(lib, iface->is_active_device,              "libcec_is_active_device",              int(CDECL *)(libcec_connection_t, CEC_NAMESPACE cec_logical_address));
  _libcecc_resolve(lib, iface->is_active_device_type,         "libcec_is_active_device_type",         int(CDECL *)(libcec_connection_t, CEC_NAMESPACE cec_device_type));
  _libcecc_resolve(lib, iface->set_hdmi_port,                 "libcec_set_hdmi_port",                 int(CDECL *)(libcec_connection_t, CEC_NAMESPACE cec_logical_address, uint8_t));
  _libcecc_resolve(lib, iface->volume_up,                     "libcec_volume_up",                     int(CDECL *)(libcec_connection_t, int));
  _libcecc_resolve(lib, iface->volume_down,                   "libcec_volume_down",                   int(CDECL *)(libcec_connection_t, int));
  _libcecc_resolve(lib, iface->mute_audio,                    "libcec_mute_audio",                    int(CDECL *)(libcec_connection_t, int));
  _libcecc_resolve(lib, iface->send_keypress,                 "libcec_send_keypress",                 int(CDECL *)(libcec_connection_t, CEC_NAMESPACE cec_logical_address, CEC_NAMESPACE cec_user_control_code, int));
  _libcecc_resolve(lib, iface->send_key_release,              "libcec_send_key_release",              int(CDECL *)(libcec_connection_t, CEC_NAMESPACE cec_logical_address, int));
  _libcecc_resolve(lib, iface->get_device_osd_name,           "libcec_get_device_osd_name",           int(CDECL *)(libcec_connection_t, CEC_NAMESPACE cec_logical_address, CEC_NAMESPACE cec_osd_name));
  _libcecc_resolve(lib, iface->set_stream_path_logical,       "libcec_set_stream_path_logical",       int(CDECL *)(libcec_connection_t, CEC_NAMESPACE cec_logical_address));
  _libcecc_resolve(lib, iface->set_stream_path_physical,      "libcec_set_stream_path_physical",      int(CDECL *)(libcec_connection_t, uint16_t));
  _libcecc_resolve(lib, iface->get_logical_addresses,         "libcec_get_logical_addresses",         CEC_NAMESPACE cec_logical_addresses(CDECL *)(libcec_connection_t));
  _libcecc_resolve(lib, iface->get_current_configuration,     "libcec_get_current_configuration",     int(CDECL *)(libcec_connection_t, CEC_NAMESPACE libcec_configuration*));
  _libcecc_resolve(lib, iface->can_persist_configuration,     "libcec_can_persist_configuration",     int(CDECL *)(libcec_connection_t));
  _libcecc_resolve(lib, iface->persist_configuration,         "libcec_persist_configuration",         int(CDECL *)(libcec_connection_t, CEC_NAMESPACE libcec_configuration*));
  _libcecc_resolve(lib, iface->set_configuration,             "libcec_set_configuration",             int(CDECL *)(libcec_connection_t, const CEC_NAMESPACE libcec_configuration*));
  _libcecc_resolve(lib, iface->rescan_devices,                "libcec_rescan_devices",                void(CDECL *)(libcec_connection_t));
  _libcecc_resolve(lib, iface->is_libcec_active_source,       "libcec_is_libcec_active_source",       int(CDECL *)(libcec_connection_t));
  _libcecc_resolve(lib, iface->get_device_information,        "libcec_get_device_information",        int(CDECL *)(libcec_connection_t, const char*, CEC_NAMESPACE libcec_configuration*, uint32_t));
  _libcecc_resolve(lib, iface->get_lib_info,                  "libcec_get_lib_info",                  const char*(CDECL *)(libcec_connection_t));
  _libcecc_resolve(lib, iface->init_video_standalone,         "libcec_init_video_standalone",         void(CDECL *)(libcec_connection_t));
  _libcecc_resolve(lib, iface->get_adapter_vendor_id,         "libcec_get_adapter_vendor_id",         uint16_t(CDECL *)(libcec_connection_t));
  _libcecc_resolve(lib, iface->get_adapter_product_id,        "libcec_get_adapter_product_id",        uint16_t(CDECL *)(libcec_connection_t));
  _libcecc_resolve(lib, iface->audio_toggle_mute,             "libcec_audio_toggle_mute",             uint8_t(CDECL *)(libcec_connection_t));
  _libcecc_resolve(lib, iface->audio_mute,                    "libcec_audio_mute",                    uint8_t(CDECL *)(libcec_connection_t));
  _libcecc_resolve(lib, iface->audio_unmute,                  "libcec_audio_unmute",                  uint8_t(CDECL *)(libcec_connection_t));
  _libcecc_resolve(lib, iface->audio_get_status,              "libcec_audio_get_status",              uint8_t(CDECL *)(libcec_connection_t));
  _libcecc_resolve(lib, iface->detect_adapters,               "libcec_detect_adapters",               int8_t(CDECL *)(libcec_connection_t, CEC_NAMESPACE cec_adapter_descriptor*, uint8_t, const char*, int));
  _libcecc_resolve(lib, iface->menu_state_to_string,          "libcec_menu_state_to_string",          void(CDECL *)(const CEC_NAMESPACE cec_menu_state, char*, size_t));
  _libcecc_resolve(lib, iface->cec_version_to_string,         "libcec_cec_version_to_string",         void(CDECL *)(const CEC_NAMESPACE cec_version, char*, size_t));
  _libcecc_resolve(lib, iface->power_status_to_string,        "libcec_power_status_to_string",        void(CDECL *)(const CEC_NAMESPACE cec_power_status, char*, size_t));
  _libcecc_resolve(lib, iface->logical_address_to_string,     "libcec_logical_address_to_string",     void(CDECL *)(const CEC_NAMESPACE cec_logical_address, char*, size_t));
  _libcecc_resolve(lib, iface->deck_control_mode_to_string,   "libcec_deck_control_mode_to_string",   void(CDECL *)(const CEC_NAMESPACE cec_deck_control_mode, char*, size_t));
  _libcecc_resolve(lib, iface->deck_status_to_string,         "libcec_deck_status_to_string",         void(CDECL *)(const CEC_NAMESPACE cec_deck_info, char*, size_t));
  _libcecc_resolve(lib, iface->opcode_to_string,              "libcec_opcode_to_string",              void(CDECL *)(const CEC_NAMESPACE cec_opcode, char*, size_t));
  _libcecc_resolve(lib, iface->system_audio_status_to_string, "libcec_system_audio_status_to_string", void(CDECL *)(const CEC_NAMESPACE cec_system_audio_status, char*, size_t));
  _libcecc_resolve(lib, iface->audio_status_to_string,        "libcec_audio_status_to_string",        void(CDECL *)(const CEC_NAMESPACE cec_audio_status, char*, size_t));
  _libcecc_resolve(lib, iface->vendor_id_to_string,           "libcec_vendor_id_to_string",           void(CDECL *)(const CEC_NAMESPACE cec_vendor_id, char*, size_t));
  _libcecc_resolve(lib, iface->user_control_key_to_string,    "libcec_user_control_key_to_string",    void(CDECL *)(const CEC_NAMESPACE cec_user_control_code, char*, size_t));
  _libcecc_resolve(lib, iface->adapter_type_to_string,        "libcec_adapter_type_to_string",        void(CDECL *)(const CEC_NAMESPACE cec_adapter_type, char*, size_t));
  _libcecc_resolve(lib, iface->version_to_string,             "libcec_version_to_string",             void(CDECL *)(uint32_t, char*, size_t));

  return 1;
}

static libcecc_lib_instance_t libcecc_load_library(const char* strLib)
{
  libcecc_lib_instance_t lib;
#if defined(_WIN32) || defined(_WIN64)
  lib = LoadLibrary(strLib ? strLib : "cec.dll");
  if (lib == NULL)
    printf("failed to load cec.dll\n");
#else
  #if defined(__APPLE__)
    lib =  dlopen(strLib ? strLib : "libcec." CEC_LIB_VERSION_MAJOR_STR ".dylib", RTLD_LAZY);
  #else
    lib = dlopen(strLib ? strLib : "libcec.so." CEC_LIB_VERSION_MAJOR_STR, RTLD_LAZY);
  #endif
  if (lib == NULL)
    printf("%s\n", dlerror());
#endif
  return lib;
}

static void libcecc_close_library(libcecc_lib_instance_t lib)
{
#if defined(_WIN32) || defined(_WIN64)
  FreeLibrary(lib);
#else
  dlclose(lib);
#endif
}

static void* libcecc_resolve(void* lib, const char* name)
{
#if defined(_WIN32) || defined(_WIN64)
  return GetProcAddress(lib, name);
#else
  return dlsym(lib, name);
#endif
}

void libcecc_reset_configuration(CEC_NAMESPACE libcec_configuration* configuration)
{
  void(CDECL * _clear_configuration)(CEC_NAMESPACE libcec_configuration*);
  libcecc_lib_instance_t lib;

  memset(configuration, 0, sizeof(CEC_NAMESPACE libcec_configuration));
  lib = libcecc_load_library(NULL);
  if (lib == NULL)
    return;

  _clear_configuration = (void(CDECL *)(CEC_NAMESPACE libcec_configuration*)) libcecc_resolve(lib, "libcec_clear_configuration");
  if (_clear_configuration)
    _clear_configuration(configuration);

  libcecc_close_library(lib);
}

/*!
 * @brief Create a new libCEC instance.
 * @param configuration The configuration to pass to libCEC
 * @param strLib The name of and/or path to libCEC
 * @return 1 when loaded, 0 if libCEC failed to initialise, -1 if methods failed to be resolved
 */
int libcecc_initialise(CEC_NAMESPACE libcec_configuration* configuration, libcec_interface_t* iface, const char* strLib)
{
  void* (CDECL *_cec_initialise)(CEC_NAMESPACE libcec_configuration*);

  libcecc_lib_instance_t lib;
  lib = libcecc_load_library(strLib);
  if (lib == NULL)
    return -1;

  _libcecc_resolve(lib, _cec_initialise, "libcec_initialise", void* (CDECL *)(CEC_NAMESPACE libcec_configuration*));

  iface->lib_instance = lib;
  iface->connection   = _cec_initialise(configuration);

  return iface->connection ?
      libcecc_resolve_all(lib, iface) :
      0;
}

/*!
 * @brief Destroy an instance of libCEC.
 * @param device The instance to destroy.
 */
void libcecc_destroy(libcec_interface_t* iface)
{
  if (iface->destroy)
    iface->destroy(iface->connection);
  libcecc_close_library(iface->lib_instance);
  memset(iface, 0, sizeof(libcec_interface_t));
}
