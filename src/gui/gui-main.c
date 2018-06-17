#include "gui.h"

static bool shouldExitApp = 0;

void gui_main_loop() {
  while(appletMainLoop() && !shouldExitApp)
  {
      //Scan all the inputs. This should be done once for each frame
      hidScanInput();

      //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
      u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

      if (kDown & KEY_A) {
        if (pair_check(&server)) {
          get_app_list(&server);
        }
      }
      else if (kDown & KEY_B) {
        if (pair_check(&server)) {
          enum platform system = platform_check(config.platform);
          if (config.debug_level > 0)
            printf("Beginning streaming on platform %s\n", platform_name(system));

          if (system == 0) {
            fprintf(stderr, "Platform '%s' not found\n", config.platform);
            break;
          }
          config.stream.supportsHevc = config.codec != CODEC_H264 && (config.codec == CODEC_HEVC || platform_supports_hevc(system));

          stream_start(&server, &config, system);
          gui_stream_loop();
          stream_stop(system);
        }
      }
      else if (kDown & KEY_X) {
        char pin[5];
//        sprintf(pin, "%d%d%d%d", (int)random() % 10, (int)random() % 10, (int)random() % 10, (int)random() % 10);
        sprintf(pin, "%d%d%d%d", 0, 0, 0, 0);
        printf("Please enter the following PIN on the target PC: %s\n", pin);
        if (gs_pair(&server, &pin[0]) != GS_OK) {
          fprintf(stderr, "Failed to pair to server: %s\n", gs_error);
        } else {
          printf("Succesfully paired\n");
        }
      }
      else if (kDown & KEY_Y) {
        if (gs_unpair(&server) != GS_OK) {
          fprintf(stderr, "Failed to unpair to server: %s\n", gs_error);
        } else {
          printf("Succesfully unpaired\n");
        }
      }
      else if (kDown & KEY_PLUS) {
        break;
      }

      SDL_RenderClear(gui.renderer);
      SDL_RenderPresent(gui.renderer);
  }
}
