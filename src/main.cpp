/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2015-2017 Iwan Timmer
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

#include <switch.h>

#include "application.h"
//#include "configuration.h"
//#include "config.h"
//#include "platform.h"

//#include "audio/audio.h"
//#include "video/video.h"
//#include "input/mapping.h"

//#include "ui/ui-main.h"

//#include <Limelight.h>

//#include <client.h>
//#include <discover.h>

#include <stdio.h>
#include <stdlib.h>
#include <memory>

//#include <stdbool.h>
//#include <string.h>
//#include <unistd.h>
//#include <sys/types.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <netdb.h>
//#include <arpa/inet.h>
//#include <openssl/ssl.h>
//#include <openssl/rand.h>
//#include <openssl/evp.h>
//#include <openssl/err.h>


int main(int argc, char* argv[]) {
    
    auto application = std::make_unique<Application>();
    application->start();
    return 0;

    // Parse the global Moonlight settings
//    config_parse(MOONLIGHT_DATA_DIR "moonlight.ini", &config);
//    config.debug_level = 2;


//  socketInitialize(&g_socketConfig);
//  setInitialize();
//  plInitialize();
//  nxlinkStdio();

//  // Initialize OpenSSL
//  SSL_library_init();
//  OpenSSL_add_all_algorithms();
//  ERR_load_crypto_strings();

//  // Seed the OpenSSL PRNG
//  size_t seedlen = 2048;
//  void *seedbuf = malloc(seedlen);
//  csrngGetRandomBytes(seedbuf, seedlen);
//  RAND_seed(seedbuf, seedlen);

//  // Parse the global Moonlight settings
//  config_parse(MOONLIGHT_DATA_DIR "moonlight.ini", &config);
//  config.debug_level = 2;

//  if (config.debug_level > 0)
//    printf("Moonlight Embedded %d.%d.%d (%s)\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, COMPILE_OPTIONS);

//  // Discover applicable streaming servers on the network
//  if (config.address == NULL) {
//    config.address = malloc(MAX_ADDRESS_SIZE);
//    if (config.address == NULL) {
//      perror("Not enough memory");
//      return 0;
//    }
//    config.address[0] = 0;
//    printf("Searching for server...\n");
//    gs_discover_server(config.address);
//    if (config.address[0] == 0) {
//      fprintf(stderr, "Autodiscovery failed. Specify an IP address next time.\n");
//      return 0;
//    }
//  }

//  // Parse configuration specific to the host at `address`
//  char host_config_file[128];
//  sprintf(host_config_file, MOONLIGHT_DATA_DIR "hosts/%s.ini", config.address);

//  if (access(host_config_file, R_OK) != -1)
//    config_parse(host_config_file, &config);

//  // Connect to the given host
//  printf("Connect to %s...\n", config.address);

//  int ret = gs_init(&server, config.address, MOONLIGHT_DATA_DIR "key", config.debug_level, config.unsupported);
//  if (ret == GS_OUT_OF_MEMORY) {
//    fprintf(stderr, "Not enough memory\n");
//  } else if (ret == GS_ERROR) {
//    fprintf(stderr, "Gamestream error: %s\n", gs_error);
//  } else if (ret == GS_INVALID) {
//    fprintf(stderr, "Invalid data received from server: %s\n", gs_error);
//  } else if (ret == GS_UNSUPPORTED_VERSION) {
//    fprintf(stderr, "Unsupported version: %s\n", gs_error);
//  } else if (ret != GS_OK) {
//    fprintf(stderr, "Can't connect to server %s, error: %s\n", config.address, gs_error);
//  }

//  if (config.debug_level > 0)
//    printf("NVIDIA %s, GFE %s (%s, %s)\n", server.gpuType, server.serverInfo.serverInfoGfeVersion, server.gsVersion, server.serverInfo.serverInfoAppVersion);

//  if (sui_init() < 0) {
//    fprintf(stderr, "Could not initialize Switch UI\n");
//  }
//  else if (ui_main_init() < 0) {
//    fprintf(stderr, "Could not initialize Moonlight Embedded UI\n");
//  }
//  else {
//    ui_main_loop();
//  }

//  ui_main_cleanup();
//  sui_cleanup();

//  plExit();
//  setExit();
//  socketExit();

  return 0;
}
