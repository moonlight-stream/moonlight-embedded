#pragma once

#include "common.h"
#include "input.h"
#include "element.h"

#define SUI_SCROLL_SPEED 15

struct _SUIScene;

typedef struct _SUIScene {
  size_t count;
  SUIElement *elements;

  SUIRect clip;
  SUIRect padded;
} SUIScene;

void sui_scene_init(SUIScene *scene);
void sui_scene_add_element(SUIScene *scene, SUIElement *element);
void sui_scene_remove_element(SUIScene *scene, SUIElement *element);
void sui_scene_update(SUIScene *scene, SUIInput *input);
void sui_scene_render(SUIScene *scene);
void sui_scene_scroll_to_element(SUIScene *scene, SUIElement *element);
void sui_scene_print(SUIScene *scene);
