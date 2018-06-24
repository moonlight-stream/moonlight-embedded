#pragma once

#include "common.h"
#include "input.h"

struct _Element;
struct _Scene;

typedef void (*Updater)(struct _Element *element, Input *input);
typedef void (*Renderer)(struct _Element *element);

typedef struct _Element {
  Rect bounds;
  Updater updater;
  Renderer renderer;
  bool fixed;

  // Used for tracking children in a scene
  struct _Element *_previous;
  struct _Element *_next;
  struct _Scene *_scene;
} Element;

void element_init(Element *element);
void element_layout_vertical(int sx, int sy, int spacing, int n, ...);
void element_layout_horizontal(int sx, int sy, int spacing, int n, ...);
