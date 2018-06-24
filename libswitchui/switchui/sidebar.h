#pragma once

#include "common.h"
#include "button.h"
#include "button-set.h"
#include "element.h"
#include "scene.h"
#include "ui.h"

#define SIDEBAR_DIVIDER         "-"
#define SIDEBAR_DIVIDER_HEIGHT  20
#define SIDEBAR_DIVIDER_COLOR   0xffd0d0d0

#define SIDEBAR_WIDTH           410
#define SIDEBAR_ITEM_WIDTH      (SIDEBAR_WIDTH - 2*MARGIN_SIDE)
#define SIDEBAR_ITEM_SPACING    5
#define SIDEBAR_COLOR           0xfff0f0f0

typedef struct _Sidebar {
  Button *buttons;
  Element *dividers;
  Element background;
  ButtonSet buttonSet;
  Scene scene;
} Sidebar;

/*
 * Initialize the menu items on the sidebar
 *
 * @param sidebar object to contain information about the sidebar
 * @param count   number of menu items to add (including dividers)
 * @param ...     list of items to have on the sidebar
 *
 * @example `sidebar_init(&sidebar, 3, "General", SIDEBAR_DIVIDER, "About")`
 */
void sidebar_init(Sidebar *sidebar, int count, ...);
int sidebar_update(Sidebar *sidebar, Input *input);
void sidebar_render(Sidebar *sidebar);
void sidebar_cleanup(Sidebar *sidebar);
