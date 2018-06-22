#include "scene.h"
#include "gui.h"

#include <SDL2/SDL2_gfxPrimitives.h>

void element_init(Element *element) {
  element->updater = NULL;
  element->renderer = NULL;
  element->_scene = NULL;
  element->_previous = NULL;
  element->_next = NULL;
}

void scene_init(Scene *scene) {
  scene->count = 0;
  scene->elements = NULL;
  scene->clip.x = 0;
  scene->clip.y = 0;
  scene->clip.width = gui.width;
  scene->clip.height = gui.height;
}

void scene_add_element(Scene *scene, Element *element) {
  if (element->_scene) {
    // Don't bother adding an element to a scene if it already was added
    if (element->_scene == scene) {
      return;
    }

    // Remove this element from the other scene
    scene_remove_element(element->_scene, element);
  }

  // Save some cycles by adding to the front of the list
  if (scene->elements) {
    scene->elements->_previous = element;
  }

  element->_previous = NULL;
  element->_next = scene->elements;
  element->_scene = scene;

  scene->elements = element;
  scene->count += 1;
}

void scene_remove_element(Scene *scene, Element *element) {
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

void scene_update(Scene *scene, Input *input) {
  // Call each scene element's Updater
  Element *element = scene->elements;
  while (element) {
    if (element->updater) {
      element->updater(element, input);
    }

    element = element->_next;
  }
}

void scene_render(Scene *scene) {
  // Call each scene element's Renderer
  Element *element = scene->elements;
  while (element) {
    if (element->renderer) {
      element->renderer(element);
    }

    element = element->_next;
  }

//  // DEBUG
//  rectangleColor(gui.renderer, scene->clip.x, scene->clip.y, scene->clip.x + scene->clip.width, scene->clip.y + scene->clip.height, RGBA8(255, 0, 0, 255));
}

void scene_scroll_to_element(Scene *scene, Element *element) {
  Rect in = intersect_bounds_clip(element->bounds.x, element->bounds.y, element->bounds.width, element->bounds.height, &scene->padded);
  int dx = 0, dy = 0;

  if (in.x != element->bounds.x || in.y != element->bounds.y) {
    // Element is clipped off the top or the left of the scene's clip
    int targetx = (int)fmaxf(element->bounds.x, scene->padded.x);
    int targety = (int)fmaxf(element->bounds.y, scene->padded.y);

    dx = (int)fminf(SCROLL_SPEED, targetx - element->bounds.x);
    dy = (int)fminf(SCROLL_SPEED, targety - element->bounds.y);
  }
  else if (in.width != element->bounds.width || in.height != element->bounds.height) {
    // Element is clipped off the bottom or right of the scene's clip
    int targetx = (int)fminf(element->bounds.x, scene->padded.x + (scene->padded.width - element->bounds.width));
    int targety = (int)fminf(element->bounds.y, scene->padded.y + (scene->padded.height - element->bounds.height));

    dx = -1 * (int)fminf(SCROLL_SPEED, element->bounds.x - targetx);
    dy = -1 * (int)fminf(SCROLL_SPEED, element->bounds.y - targety);
  }

  if (dx || dy) {
    // Update the location of every element in the scene by the delta
    Element *element = scene->elements;
    while (element) {
      element->bounds.x += dx;
      element->bounds.y += dy;
      element = element->_next;
    }
  }
}

void scene_print(Scene *scene) {
  Element *element = scene->elements;
  while (element) {
    element = element->_next;
  }
}
