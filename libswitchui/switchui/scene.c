#include "scene.h"
#include "ui.h"

#include <SDL2/SDL2_gfxPrimitives.h>

void sui_scene_init(SUIScene *scene) {
  scene->count = 0;
  scene->elements = NULL;
  scene->clip.x = 0;
  scene->clip.y = 0;
  scene->clip.w = ui.width;
  scene->clip.h = ui.height;
}

void sui_scene_add_element(SUIScene *scene, SUIElement *element) {
  if (element->_scene) {
    // Don't bother adding an element to a scene if it already was added
    if (element->_scene == scene) {
      return;
    }

    // Remove this element from the other scene
    sui_scene_remove_element(element->_scene, element);
  }

  if (scene->elements == NULL) {
    scene->elements = element;
    element->_previous = NULL;
  }
  else {
    // Find the last element in the scene
    SUIElement *end = scene->elements;
    while (end->_next) {
      end = end->_next;
    }

    end->_next = element;
    element->_previous = end;
  }

  element->_next = NULL;
  element->_scene = scene;
  scene->count += 1;
}

void sui_scene_remove_element(SUIScene *scene, SUIElement *element) {
  if (element->_scene != scene) {
    return;
  }

  if (element->_previous) {
    element->_previous->_next = element->_next;
  }
  else {
    // If there was no previous, this was the head of the scene
    scene->elements = element->_next;
  }

  if (element->_next) {
    element->_next->_previous = element->_previous;
  }

  element->_previous = NULL;
  element->_next = NULL;
  element->_scene = NULL;
}

void sui_scene_update(SUIScene *scene, SUIInput *input) {
  // Call each scene element's Updater
  SUIElement *element = scene->elements;
  while (element) {
    if (element->updater) {
      element->updater(element, input);
    }

    element = element->_next;
  }
}

void sui_scene_render(SUIScene *scene) {
  // Call each scene element's Renderer
  SUIElement *element = scene->elements;
  while (element) {
    if (element->renderer) {
      element->renderer(element);
    }

    element = element->_next;
  }

//  // DEBUG
//  rectangleColor(ui.renderer, scene->clip.x, scene->clip.y, scene->clip.x + scene->clip.width, scene->clip.y + scene->clip.height, RGBA8(255, 0, 0, 255));
}

void sui_scene_scroll_to_element(SUIScene *scene, SUIElement *element) {
  SUIRect in = sui_intersect_bounds_clip(element->bounds.x, element->bounds.y, element->bounds.w, element->bounds.h, &scene->padded);
  int dx = 0, dy = 0;

  if (in.x != element->bounds.x || in.y != element->bounds.y) {
    // Element is clipped off the top or the left of the scene's clip
    int targetx = (int)fmaxf(element->bounds.x, scene->padded.x);
    int targety = (int)fmaxf(element->bounds.y, scene->padded.y);

    dx = (int)fminf(SUI_SCROLL_SPEED, targetx - element->bounds.x);
    dy = (int)fminf(SUI_SCROLL_SPEED, targety - element->bounds.y);
  }
  else if (in.w != element->bounds.w || in.h != element->bounds.h) {
    // Element is clipped off the bottom or right of the scene's clip
    int targetx = (int)fminf(element->bounds.x, scene->padded.x + (scene->padded.w - element->bounds.w));
    int targety = (int)fminf(element->bounds.y, scene->padded.y + (scene->padded.h - element->bounds.h));

    dx = -1 * (int)fminf(SUI_SCROLL_SPEED, element->bounds.x - targetx);
    dy = -1 * (int)fminf(SUI_SCROLL_SPEED, element->bounds.y - targety);
  }

  if (dx || dy) {
    // Update the location of every element in the scene by the delta
    SUIElement *element = scene->elements;
    while (element) {
      if (!element->fixed) {
        element->bounds.x += dx;
        element->bounds.y += dy;
      }

      element = element->_next;
    }
  }
}

void sui_scene_print(SUIScene *scene) {
  SUIElement *element = scene->elements;
  while (element) {
    element = element->_next;
  }
}
