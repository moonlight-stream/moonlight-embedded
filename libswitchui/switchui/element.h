#pragma once

#include "common.h"
#include "input.h"

struct _SUIElement;
struct _SUIScene;

typedef void (*SUIUpdater)(struct _SUIElement *element, SUIInput *input);
typedef void (*SUIRenderer)(struct _SUIElement *element);

typedef struct _SUIElement {
  SUIRect bounds;
  SUIUpdater updater;
  SUIRenderer renderer;
  bool fixed;

  // Used for tracking children in a scene
  struct _SUIElement *_previous;
  struct _SUIElement *_next;
  struct _SUIScene *_scene;
} SUIElement;

void sui_element_init(SUIElement *element);
void sui_element_layout_vertical(int sx, int sy, int spacing, int n, ...);
void sui_element_layout_horizontal(int sx, int sy, int spacing, int n, ...);
