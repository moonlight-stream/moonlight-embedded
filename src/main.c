/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2015, 2016 Iwan Timmer
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

#include "loop.h"
#include "client.h"
#include "connection.h"
#include "configuration.h"
#include "audio.h"
#include "video.h"
#include "discover.h"
#include "config.h"
#include "platform.h"
#include "sdl.h"

#include "input/evdev.h"
#include "input/udev.h"
#include "input/cec.h"
#include "input/sdlinput.h"
#include "input/vita.h"

#include <Limelight.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <ctype.h>

#include <psp2/net/net.h>
#include <psp2/sysmodule.h>

#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/rtc.h>

#include "graphics.h"
#include "gui/ui.h"

static void help() {
  printf("Usage: moonlight [action] (options) [host]\n");
  printf("       moonlight [configfile]\n");
  printf("\n Actions\n\n");
  printf("\tmap\t\t\tCreate mapping file for gamepad\n");
  printf("\tpair\t\t\tPair device with computer\n");
  printf("\tunpair\t\t\tUnpair device with computer\n");
  printf("\tstream\t\t\tStream computer to device\n");
  printf("\tlist\t\t\tList available games and applications\n");
  printf("\tquit\t\t\tQuit the application or game being streamed\n");
  printf("\thelp\t\t\tShow this help\n");
  printf("\n Global Options\n\n");
  printf("\t-config <config>\tLoad configuration file\n");
  printf("\t-save <config>\t\tSave configuration file\n");
  printf("\n Streaming options\n\n");
  printf("\t-720\t\t\tUse 1280x720 resolution [default]\n");
  printf("\t-1080\t\t\tUse 1920x1080 resolution\n");
  printf("\t-width <width>\t\tHorizontal resolution (default 1280)\n");
  printf("\t-height <height>\tVertical resolution (default 720)\n");
  printf("\t-30fps\t\t\tUse 30fps\n");
  printf("\t-60fps\t\t\tUse 60fps [default]\n");
  printf("\t-bitrate <bitrate>\tSpecify the bitrate in Kbps\n");
  printf("\t-packetsize <size>\tSpecify the maximum packetsize in bytes\n");
  printf("\t-forcehevc\t\tUse high efficiency video decoding (HEVC)\n");
  printf("\t-remote\t\t\tEnable remote optimizations\n");
  printf("\t-app <app>\t\tName of app to stream\n");
  printf("\t-nosops\t\t\tDon't allow GFE to modify game settings\n");
  printf("\t-localaudio\t\tPlay audio locally\n");
  printf("\t-surround\t\tStream 5.1 surround sound (requires GFE 2.7)\n");
  printf("\t-keydir <directory>\tLoad encryption keys from directory\n");
  #ifdef HAVE_SDL
  printf("\n Video options (SDL Only)\n\n");
  printf("\t-windowed\t\tDisplay screen in a window\n");
  #endif
  #ifdef HAVE_EMBEDDED
  printf("\n I/O options\n\n");
  printf("\t-mapping <file>\t\tUse <file> as gamepad mapping configuration file (use before -input)\n");
  printf("\t-input <device>\t\tUse <device> as input. Can be used multiple times\n");
  printf("\t-audio <device>\t\tUse <device> as audio output device\n");
  printf("\t-forcehw \t\tTry to use video hardware acceleration\n");
  #endif
  printf("\nUse Ctrl+Alt+Shift+Q to exit streaming session\n\n");
  exit(0);
}

static void vita_init() {
  // Seed OpenSSL with Sony-grade random number generator
  char random_seed[0x40] = {0};
  sceKernelGetRandomNumber(random_seed, sizeof(random_seed));
  RAND_seed(random_seed, sizeof(random_seed));
  OpenSSL_add_all_algorithms();

  // This is only used for PIN codes, doesn't really matter
  srand(time(NULL));

  printf("Moonlight Embedded %d.%d.%d (%s)\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, COMPILE_OPTIONS);

  int ret = 0;

  ret = sceSysmoduleLoadModule(SCE_SYSMODULE_NET);

  size_t net_mem_sz = 100 * 1024;
  SceNetInitParam net_param = {0};
  net_param.memory = calloc(net_mem_sz, 1);
  net_param.size = net_mem_sz;
  ret = sceNetInit(&net_param);

  ret = sceNetCtlInit();
  // TODO(xyz): cURL breaks when socket FD is too big, very hacky workaround below!
  int s = sceNetSocket("", SCE_NET_AF_INET, SCE_NET_SOCK_STREAM, 0);
  sceNetSocketClose(s);
  if (s >= 20) {
    printf("Cycling sockets...\n");
    int c = 0;
    do {
      c = sceNetSocket("", SCE_NET_AF_INET, SCE_NET_SOCK_STREAM, 0);
      sceNetSocketClose(c);
    } while (c >= 5);
  }
}

void loop_forever(void) {
  while (connection_is_ready()) {
    sceKernelDelayThread(100 * 1000);
  }
}

int main(int argc, char* argv[]) {
  psvDebugScreenInit();
  vita_init();
  if (!vitapower_init()) {
    printf("Failed to init power!");
    loop_forever();
  }

  if (!vitainput_init()) {
    printf("Failed to init input!");
    loop_forever();
  }

  config_path = "ux0:data/moonlight/moonlight.conf";
  config_parse(argc, argv, &config);
  config.platform = "vita";
  strcpy(config.key_dir, "ux0:data/moonlight/");

  vitapower_config(config);
  vitainput_config(config);

  gui_loop();

  /*
  int ret = 0;

   // cURL socket bug reprod
  SERVER_DATA server;
  server.address = "192.168.12.252";

  gs_init(&server, config.key_dir);

  while (true) {
    __stream(&server, 999999);
    sceKernelDelayThread(2000 * 1000);
  }
  */


  /*
   *
again:
  printf("Press X to pair (You need to do it once)\n");
  printf("Press O to launch steam\n");

  connection_reset();

  switch(get_key()) {
  case SCE_CTRL_CROSS:
    vita_pair(&server);
    break;
  case SCE_CTRL_CIRCLE:
    stream(&server, &config, system);
    loop_forever();
    break;
  }
<<<<<<< HEAD
  */
}

 //cURL sockets bug reproduction
void __stream(PSERVER_DATA server, int appId) {
  int ret;

  ret = sceNetCtlInit();
  ret = gs_start_app(server, &config.stream, appId, config.sops, config.localaudio);
  if (ret < 0) {
    if (ret == GS_NOT_SUPPORTED_4K)
      printf("Server doesn't support 4K\n");
    else
      printf("Errorcode starting app: %d\n", ret);

    return;
  }

  enum platform system = VITA;
  int drFlags = 0;

  if (config.fullscreen)
    drFlags |= DISPLAY_FULLSCREEN;
  if (config.forcehw)
    drFlags |= FORCE_HARDWARE_ACCELERATION;

  printf("\nStart connection...\n");
  ret = LiStartConnection(
      server->address,
      &config.stream,
      &connection_callbacks,
      platform_get_video(system),
      platform_get_audio(system),
      NULL,
      drFlags,
      server->serverMajorVersion
      );
  if (ret == 0) {
    //vitainput_loop();
    LiStopConnection();
  } else {
    printf("\nFAILED CONNECTING\n");
  }
}
