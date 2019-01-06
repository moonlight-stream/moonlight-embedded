// #include "ui-main.h"

// #include "moonlight_switch_logo_png.h"

// static struct {
//   SDL_Texture *logoTexture;
//   int logoWidth;
//   int logoHeight;

//   SUIButton connectButton;
//   SUIButton settingsButton;
//   SUIButtonSet buttons;

//   FpsCounter counter;
// } props = {0};

// int main_init_initial() {
//   props.logoTexture = sui_load_png(moonlight_switch_logo_png, moonlight_switch_logo_png_size);
//   if (!props.logoTexture) {
//     fprintf(stderr, "[GUI, initial] Could not load logo: %s\n", SDL_GetError());
//     return -1;
//   }
//   SDL_QueryTexture(props.logoTexture, NULL, NULL, &props.logoWidth, &props.logoHeight);

//   sui_button_init(&props.connectButton);
//   props.connectButton.text = "Connect";
//   props.connectButton.e.bounds.x = ui.width/2 - props.connectButton.e.bounds.w/2;
//   props.connectButton.e.bounds.y = 100 + props.logoHeight + (ui.height - SUI_MARGIN_BOTTOM - props.logoHeight - 100)/2 - props.connectButton.e.bounds.h/2;
//   props.connectButton.focused = true;

//   sui_button_init(&props.settingsButton);
//   props.settingsButton.text = "Settings";
//   props.settingsButton.e.bounds.x = ui.width/2 - props.settingsButton.e.bounds.w/2;
//   props.settingsButton.e.bounds.y = props.connectButton.e.bounds.y + props.connectButton.e.bounds.h + 15;
//   props.settingsButton.focused = false;

//   sui_button_set_init(&props.buttons, Vertical);
//   sui_button_set_add(&props.buttons, &props.connectButton);
//   sui_button_set_add(&props.buttons, &props.settingsButton);

//   return 0;
// }

// void main_update_initial(SUIInput *input) {
//   SUIButton *clicked = sui_button_set_update(&props.buttons, input, NULL);

//   if (clicked == &props.connectButton) {
//     ui_state = sui_state_push(ui_state, &MoonlightUiStateConnecting);
//   }
//   else if (clicked == &props.settingsButton) {
//     ui_state = sui_state_push(ui_state, &MoonlightUiStateSettings);
//   }
//   else if (input->buttons.down & KEY_B) {
//     ui_shouldExitApp = true;
//   }

//   ui_update_fps(&props.counter);
// }

// void main_render_initial() {
//   SDL_SetRenderDrawColor(ui.renderer, 0xeb, 0xeb, 0xeb, 0xff);
//   SDL_RenderClear(ui.renderer);

//   // Draw the logo
//   sui_draw_texture(props.logoTexture, (ui.width - props.logoWidth) / 2, 150, props.logoWidth, props.logoHeight);

//   // Draw the OK action on the bottom toolbar
//   sui_draw_bottom_toolbar(1, "OK", SUIToolbarActionA);

//   // Draw the main buttons
//   sui_button_set_render(&props.buttons);

//   ui_draw_fps(&props.counter);

//   SDL_RenderPresent(ui.renderer);
// }

// void main_cleanup_initial() {
//   if (props.logoTexture) {
//     SDL_DestroyTexture(props.logoTexture);
//     props.logoTexture = NULL;
//   }
// }

// MoonlightUiState MoonlightUiStateInitial = {
//   .state = 0,
//   .init = &main_init_initial,
//   .update = &main_update_initial,
//   .render = &main_render_initial,
//   .cleanup = &main_cleanup_initial
// };
