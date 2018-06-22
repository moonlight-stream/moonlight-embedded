#pragma once

#include "../input/switch.h"

#define SCROLL_SPEED 15

struct _Element;
struct _Scene;

typedef struct _Rect {
  int x;
  int y;
  int width;
  int height;
} Rect;

typedef void (*Updater)(struct _Element *element, Input *input);
typedef void (*Renderer)(struct _Element *element);

typedef struct _Element {
  Rect bounds;
  Updater updater;
  Renderer renderer;

  // Used for tracking children in a scene
  struct _Element *_previous;
  struct _Element *_next;
  struct _Scene *_scene;
} Element;

typedef struct _Scene {
  size_t count;
  Element *elements;

  Rect clip;
  Rect padded;
} Scene;

void element_init(Element *element);

void scene_init(Scene *scene);
void scene_add_element(Scene *scene, Element *element);
void scene_remove_element(Scene *scene, Element *element);
void scene_update(Scene *scene, Input *input);
void scene_render(Scene *scene);
void scene_scroll_to_element(Scene *scene, Element *element);
void scene_print(Scene *scene);
