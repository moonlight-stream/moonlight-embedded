// #include "ui-main.h"

// #include "moonlight_switch_connecting_png.h"

// static struct {
//   SDL_Texture *connectingTexture;
//   int connectingWidth;
//   int connectingHeight;

//   FpsCounter counter;
//   char pin[5];
// } props = {0};

// int main_init_connecting() {
//   props.connectingTexture = sui_load_png(moonlight_switch_connecting_png, moonlight_switch_connecting_png_size);
//   if (!props.connectingTexture) {
//     fprintf(stderr, "[GUI] Could not load connecting image: %s\n", SDL_GetError());
//     return -1;
//   }
//   SDL_QueryTexture(props.connectingTexture, NULL, NULL, &props.connectingWidth, &props.connectingHeight);

//   return 0;
// }

// void main_update_connecting(SUIInput *input) {
//   if (props.counter.frame == 0) {
// //    sprintf(props.pin, "%d%d%d%d", (int)random() % 10, (int)random() % 10, (int)random() % 10, (int)random() % 10);
//     sprintf(props.pin, "0000");
//   }
//   else {
//     if (gs_pair(&server, &props.pin[0]) == GS_OK) {
//       ui_state = sui_state_replace(ui_state, &MoonlightUiStateGamesList);
//     }
//     else {
//       ui_state = sui_state_replace(ui_state, &MoonlightUiStateConnectionFailed);
//     }

//     if (input->buttons.down & KEY_B) {
//       ui_state = sui_state_pop(ui_state);
//     }
//   }

//   ui_update_fps(&props.counter);
// }


// void main_render_connecting()  {
//   SDL_SetRenderDrawColor(ui.renderer, 0xeb, 0xeb, 0xeb, 0xff);
//   SDL_RenderClear(ui.renderer);

//   // Draw the connecting image
//   sui_draw_texture(props.connectingTexture, (ui.width - props.connectingWidth) / 2, SUI_MARGIN_TOP + 75, props.connectingWidth, props.connectingHeight);

//   // Draw the guide text
//   int textNormalHeight = sui_text_ascent(ui.fontNormal);
//   int textEnterWidth, textEnterX, textEnterY;
//   char *textEnter = "Enter the following PIN on the target PC:";
//   sui_measure_text(ui.fontNormal, textEnter, &textEnterWidth, NULL);
//   textEnterX = (ui.width - textEnterWidth) / 2;
//   textEnterY = SUI_MARGIN_TOP + 75 + props.connectingHeight + 90;
//   sui_draw_text(ui.fontNormal, textEnter, textEnterX, textEnterY, SUI_COLOR_DARK, false, -1);

//   // Draw the PIN text
//   int textMassiveHeight = sui_text_ascent(ui.fontHeading);
//   int textPinWidth, textPinX, textPinY;
//   sui_measure_text(ui.fontMassive, props.pin, &textPinWidth, NULL);
//   textPinX = (ui.width - textPinWidth) / 2;
//   textPinY = SUI_MARGIN_TOP + 75 + props.connectingHeight + 90 + textNormalHeight + 60;
//   sui_draw_text(ui.fontMassive, props.pin, textPinX, textPinY, SUI_COLOR_DARK, false, -1);

//   // Draw the heading
//   sui_draw_top_header("Moonlight  ›  Connection");

//   // Draw the OK and Back actions on the bottom toolbar
//   sui_draw_bottom_toolbar(2, "OK", SUIToolbarAction::A, "Back", SUIToolbarAction::B);

//   ui_draw_fps(&props.counter);

//   SDL_RenderPresent(ui.renderer);
// }

// void main_cleanup_connecting() {
//   if (props.connectingTexture) {
//     SDL_DestroyTexture(props.connectingTexture);
//     props.connectingTexture = NULL;
//   }
// }

// MoonlightUiState MoonlightUiStateConnecting = {
//   .state = 2,
//   .init = &main_init_connecting,
//   .update = &main_update_connecting,
//   .render = &main_render_connecting,
//   .cleanup = &main_cleanup_connecting
// };
