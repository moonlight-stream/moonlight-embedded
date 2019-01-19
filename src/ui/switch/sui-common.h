#pragma once

#include <switch.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL2_rotozoom.h>

#include <initializer_list>
#include <tuple>
#include <utility>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <numeric>
#include <functional>

typedef SDL_Rect SUIRect;

enum SUIFocusResult {
    SUIFocusRetain,
    SUIFocusRelease
};

enum SUIEvent {
    SUIEventClick,
    SUIEventFocus
};

class SUIElement;
typedef std::function<void(SUIElement *, SUIEvent)> SUIEventListener;

typedef struct {
    struct {
        uint64_t down;
        uint64_t held;
        uint64_t up;
    } buttons;

    struct {
        JoystickPosition left;
        JoystickPosition right;
    } joysticks;

    struct {
        bool touched;
        touchPosition position;
    } touch;
} SUIInput;