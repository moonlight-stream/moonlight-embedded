#include "sidebar.h"

static void sidebar_divider_renderer(SUIElement *element) {
  hlineColor(ui.renderer,
             element->bounds.x,
             element->bounds.x + element->bounds.w,
             element->bounds.y + element->bounds.h/2,
             SUI_SIDEBAR_DIVIDER_COLOR);
}

static void sidebar_background_renderer(SUIElement *element) {
  boxColor(ui.renderer,
           element->bounds.x,
           element->bounds.y,
           element->bounds.x + element->bounds.w,
           element->bounds.y + element->bounds.h,
           SUI_SIDEBAR_COLOR);
}

static void sidebar_scene_init(SUISidebar *sidebar) {
  sui_scene_init(&sidebar->scene);
  sidebar->scene.clip.x = 0;
  sidebar->scene.clip.y = SUI_MARGIN_TOP + 1;
  sidebar->scene.clip.w = SUI_SIDEBAR_WIDTH;
  sidebar->scene.clip.h = ui.height - SUI_MARGIN_TOP - SUI_MARGIN_BOTTOM - 2;
  sidebar->scene.padded.x = SUI_MARGIN_SIDE;
  sidebar->scene.padded.y = SUI_MARGIN_TOP + SUI_MARGIN_SIDE + 1;
  sidebar->scene.padded.w = SUI_SIDEBAR_WIDTH - 2*SUI_MARGIN_SIDE;
  sidebar->scene.padded.h = ui.width - SUI_MARGIN_TOP - SUI_MARGIN_BOTTOM - 2*SUI_MARGIN_SIDE - 2;
}

void sui_sidebar_init(SUISidebar *sidebar, int count, ...) {
  int i;
  va_list arg, counter;
  va_start(arg, count);

  // Count the number of dividers and number of buttons
  int nbuttons = 0, ndividers = 0;
  int cbutton = 0, cdivider = 0;

  va_copy(counter, arg);
  for (i = 0; i < count; i++) {
    char *text = va_arg(counter, char *);

    if (strcmp(text, SUI_SIDEBAR_DIVIDER) != 0) {
      nbuttons++;
    }
    else {
      ndividers++;
    }
  }
  va_end(counter);

  // Allocate space for the menu buttons and dividers
  sidebar->buttons = malloc(nbuttons * sizeof(SUIButton));
  sidebar->dividers = malloc(ndividers * sizeof(SUIElement));

  sidebar_scene_init(sidebar);
  sui_button_set_init(&sidebar->buttonSet, Vertical);
  sidebar->buttonSet.wrap = false;

  // Create the background element
  sui_element_init(&sidebar->background);
  sidebar->background.fixed = true;
  sidebar->background.bounds = sidebar->scene.clip;
  sidebar->background.renderer = &sidebar_background_renderer;
  sui_scene_add_element(&sidebar->scene, &sidebar->background);

  // Create and position each item
  int sx = SUI_MARGIN_SIDE;
  int sy = SUI_MARGIN_TOP + SUI_MARGIN_SIDE;

  for (i = 0; i < count; i++) {
    SUIElement *item;
    char *text = va_arg(arg, char *);

    if (strcmp(text, SUI_SIDEBAR_DIVIDER) != 0) {
      SUIButton *button = &(sidebar->buttons[cbutton]);
      sui_button_init(button);
      button->text = text;
      button->contentRenderer = &sui_button_renderer_content_menu;
      button->user = cbutton;
      button->e.bounds.w = SUI_SIDEBAR_ITEM_WIDTH;
      button->e.bounds.h = SUI_BUTTON_DEFAULT_HEIGHT;
      sui_button_set_add(&sidebar->buttonSet, button);

      cbutton++;
      item = (SUIElement *)button;
    }
    else {
      SUIElement *divider = &(sidebar->dividers[cdivider]);
      sui_element_init(divider);
      divider->renderer = &sidebar_divider_renderer;
      divider->bounds.w = SUI_SIDEBAR_ITEM_WIDTH;
      divider->bounds.h = SUI_SIDEBAR_DIVIDER_HEIGHT;

      cdivider++;
      item = divider;
    }

    // Position the item and add it to the scene
    item->bounds.x = sx;
    item->bounds.y = sy;
    sui_scene_add_element(&sidebar->scene, item);

    sy = sy + item->bounds.h + SUI_SIDEBAR_ITEM_SPACING;
  }

  va_end(arg);
}

int sui_sidebar_update(SUISidebar *sidebar, SUIInput *input) {
  // Update the scene elements
  sui_scene_update(&sidebar->scene, input);

  // Update the menu item buttons from the input
  SUIButton *clicked = sui_button_set_update(&sidebar->buttonSet, input, NULL);

  if (clicked) {
    return (int)clicked->user;
  }

  return -1;
}

void sui_sidebar_render(SUISidebar *sidebar) {
  sui_scene_render(&sidebar->scene);
}

void sui_sidebar_cleanup(SUISidebar *sidebar) {
  free(sidebar->buttons);
  free(sidebar->dividers);
}
