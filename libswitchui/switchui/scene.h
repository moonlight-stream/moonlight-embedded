#pragma once

#include "common.h"
#include "input.h"
#include "element.h"

#define SCROLL_SPEED 15

struct _Scene;

typedef struct _Scene {
  size_t count;
  Element *elements;

  Rect clip;
  Rect padded;
} Scene;

void scene_init(Scene *scene);
void scene_add_element(Scene *scene, Element *element);
void scene_remove_element(Scene *scene, Element *element);
void scene_update(Scene *scene, Input *input);
void scene_render(Scene *scene);
void scene_scroll_to_element(Scene *scene, Element *element);
void scene_print(Scene *scene);
