#include "sidebar.h"

static void sidebar_divider_renderer(Element *element) {
  hlineColor(ui.renderer,
             element->bounds.x,
             element->bounds.x + element->bounds.w,
             element->bounds.y + element->bounds.h/2,
             SIDEBAR_DIVIDER_COLOR);
}

static void sidebar_background_renderer(Element *element) {
  boxColor(ui.renderer,
           element->bounds.x,
           element->bounds.y,
           element->bounds.x + element->bounds.w,
           element->bounds.y + element->bounds.h,
           SIDEBAR_COLOR);
}

static void sidebar_scene_init(Sidebar *sidebar) {
  scene_init(&sidebar->scene);
  sidebar->scene.clip.x = 0;
  sidebar->scene.clip.y = MARGIN_TOP + 1;
  sidebar->scene.clip.w = SIDEBAR_WIDTH;
  sidebar->scene.clip.h = ui.height - MARGIN_TOP - MARGIN_BOTTOM - 2;
  sidebar->scene.padded.x = MARGIN_SIDE;
  sidebar->scene.padded.y = MARGIN_TOP + MARGIN_SIDE + 1;
  sidebar->scene.padded.w = SIDEBAR_WIDTH - 2*MARGIN_SIDE;
  sidebar->scene.padded.h = ui.width - MARGIN_TOP - MARGIN_BOTTOM - 2*MARGIN_SIDE - 2;
}

void sidebar_init(Sidebar *sidebar, int count, ...) {
  int i;
  va_list arg, counter;
  va_start(arg, count);

  // Count the number of dividers and number of buttons
  int nbuttons = 0, ndividers = 0;
  int cbutton = 0, cdivider = 0;

  va_copy(counter, arg);
  for (i = 0; i < count; i++) {
    char *text = va_arg(counter, char *);

    if (strcmp(text, SIDEBAR_DIVIDER) != 0) {
      nbuttons++;
    }
    else {
      ndividers++;
    }
  }
  va_end(counter);

  // Allocate space for the menu buttons and dividers
  sidebar->buttons = malloc(nbuttons * sizeof(Button));
  sidebar->dividers = malloc(ndividers * sizeof(Element));

  sidebar_scene_init(sidebar);
  button_set_init(&sidebar->buttonSet, Vertical);
  sidebar->buttonSet.wrap = false;

  // Create the background element
  element_init(&sidebar->background);
  sidebar->background.fixed = true;
  sidebar->background.bounds = sidebar->scene.clip;
  sidebar->background.renderer = &sidebar_background_renderer;
  scene_add_element(&sidebar->scene, &sidebar->background);

  // Create and position each item
  int sx = MARGIN_SIDE;
  int sy = MARGIN_TOP + MARGIN_SIDE;

  for (i = 0; i < count; i++) {
    Element *item;
    char *text = va_arg(arg, char *);

    if (strcmp(text, SIDEBAR_DIVIDER) != 0) {
      Button *button = &(sidebar->buttons[cbutton]);
      button_init(button);
      button->text = text;
      button->contentRenderer = &button_renderer_content_menu;
      button->user = cbutton;
      button->e.bounds.w = SIDEBAR_ITEM_WIDTH;
      button->e.bounds.h = BUTTON_DEFAULT_HEIGHT;

      button_set_add(&sidebar->buttonSet, button);

      cbutton++;
      item = (Element *)button;
    }
    else {
      Element *divider = &(sidebar->dividers[cdivider]);
      element_init(divider);
      divider->renderer = &sidebar_divider_renderer;
      divider->bounds.w = SIDEBAR_ITEM_WIDTH;
      divider->bounds.h = SIDEBAR_DIVIDER_HEIGHT;

      cdivider++;
      item = divider;
    }

    // Position the item and add it to the scene
    item->bounds.x = sx;
    item->bounds.y = sy;
    scene_add_element(&sidebar->scene, item);

    sy = sy + item->bounds.h + SIDEBAR_ITEM_SPACING;
  }

  va_end(arg);
}

int sidebar_update(Sidebar *sidebar, Input *input) {
  // Update the scene elements
  scene_update(&sidebar->scene, input);

  // Update the menu item buttons from the input
  Button *clicked = button_set_update(&sidebar->buttonSet, input, NULL);

  if (clicked) {
    return (int)clicked->user;
  }

  return -1;
}

void sidebar_render(Sidebar *sidebar) {
  scene_render(&sidebar->scene);
}

void sidebar_cleanup(Sidebar *sidebar) {
  free(sidebar->buttons);
  free(sidebar->dividers);
}
