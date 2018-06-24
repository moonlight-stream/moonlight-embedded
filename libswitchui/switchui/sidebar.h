#pragma once

#include "common.h"
#include "button.h"
#include "button-set.h"
#include "element.h"
#include "scene.h"
#include "ui.h"

#define SUI_SIDEBAR_DIVIDER         "-"
#define SUI_SIDEBAR_DIVIDER_HEIGHT  20
#define SUI_SIDEBAR_DIVIDER_COLOR   0xffd0d0d0

#define SUI_SIDEBAR_WIDTH           410
#define SUI_SIDEBAR_ITEM_WIDTH      (SUI_SIDEBAR_WIDTH - 2*SUI_MARGIN_SIDE)
#define SUI_SIDEBAR_ITEM_SPACING    5
#define SUI_SIDEBAR_COLOR           0xfff0f0f0

typedef struct _SUISidebar {
  SUIButton *buttons;
  SUIElement *dividers;
  SUIElement background;
  SUIButtonSet buttonSet;
  SUIScene scene;
} SUISidebar;

/*
 * Initialize the menu items on the sidebar
 *
 * @param sidebar object to contain information about the sidebar
 * @param count   number of menu items to add (including dividers)
 * @param ...     list of items to have on the sidebar
 *
 * @example `sidebar_init(&sidebar, 3, "General", SIDEBAR_DIVIDER, "About")`
 */
void sui_sidebar_init(SUISidebar *sidebar, int count, ...);
int sui_sidebar_update(SUISidebar *sidebar, SUIInput *input);
void sui_sidebar_render(SUISidebar *sidebar);
void sui_sidebar_cleanup(SUISidebar *sidebar);
