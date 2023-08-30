/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2015-2017 Iwan Timmer
 *
 * Moonlight is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Moonlight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Moonlight; if not, see <http://www.gnu.org/licenses/>.
 */

#include "sdl.h"
#include "../sdl.h"

#include <Limelight.h>

#define ACTION_MODIFIERS (MODIFIER_SHIFT|MODIFIER_ALT|MODIFIER_CTRL)
#define QUIT_KEY SDLK_q
#define QUIT_BUTTONS (PLAY_FLAG|BACK_FLAG|LB_FLAG|RB_FLAG)
#define FULLSCREEN_KEY SDLK_f
#define UNGRAB_KEY SDLK_z

static const int SDL_TO_LI_BUTTON_MAP[] = {
  A_FLAG, B_FLAG, X_FLAG, Y_FLAG,
  BACK_FLAG, SPECIAL_FLAG, PLAY_FLAG,
  LS_CLK_FLAG, RS_CLK_FLAG,
  LB_FLAG, RB_FLAG,
  UP_FLAG, DOWN_FLAG, LEFT_FLAG, RIGHT_FLAG,
  MISC_FLAG,
  PADDLE1_FLAG, PADDLE2_FLAG, PADDLE3_FLAG, PADDLE4_FLAG,
  TOUCHPAD_FLAG,
};

typedef struct _GAMEPAD_STATE {
  unsigned char leftTrigger, rightTrigger;
  short leftStickX, leftStickY;
  short rightStickX, rightStickY;
  int buttons;
  SDL_JoystickID sdl_id;
  SDL_GameController* controller;
#if !SDL_VERSION_ATLEAST(2, 0, 9)
  SDL_Haptic* haptic;
  int haptic_effect_id;
#endif
  short id;
  bool initialized;
} GAMEPAD_STATE, *PGAMEPAD_STATE;

// Limited by number of bits in activeGamepadMask
#define MAX_GAMEPADS 16

static GAMEPAD_STATE gamepads[MAX_GAMEPADS];

static int keyboard_modifiers;
static int activeGamepadMask = 0;

int sdl_gamepads = 0;

static void send_controller_arrival(PGAMEPAD_STATE state) {
#if SDL_VERSION_ATLEAST(2, 0, 18)
  unsigned int supportedButtonFlags = 0;
  unsigned short capabilities = 0;
  unsigned char type = LI_CTYPE_UNKNOWN;

  for (int i = 0; i < SDL_arraysize(SDL_TO_LI_BUTTON_MAP); i++) {
    if (SDL_GameControllerHasButton(state->controller, (SDL_GameControllerButton)i)) {
        supportedButtonFlags |= SDL_TO_LI_BUTTON_MAP[i];
    }
  }

  if (SDL_GameControllerGetBindForAxis(state->controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT).bindType == SDL_CONTROLLER_BINDTYPE_AXIS ||
      SDL_GameControllerGetBindForAxis(state->controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT).bindType == SDL_CONTROLLER_BINDTYPE_AXIS)
    capabilities |= LI_CCAP_ANALOG_TRIGGERS;
  if (SDL_GameControllerHasRumble(state->controller))
    capabilities |= LI_CCAP_RUMBLE;
  if (SDL_GameControllerHasRumbleTriggers(state->controller))
    capabilities |= LI_CCAP_TRIGGER_RUMBLE;
  if (SDL_GameControllerGetNumTouchpads(state->controller) > 0)
    capabilities |= LI_CCAP_TOUCHPAD;
  if (SDL_GameControllerHasSensor(state->controller, SDL_SENSOR_ACCEL))
    capabilities |= LI_CCAP_ACCEL;
  if (SDL_GameControllerHasSensor(state->controller, SDL_SENSOR_GYRO))
    capabilities |= LI_CCAP_GYRO;
  if (SDL_GameControllerHasLED(state->controller))
    capabilities |= LI_CCAP_RGB_LED;

  switch (SDL_GameControllerGetType(state->controller)) {
  case SDL_CONTROLLER_TYPE_XBOX360:
  case SDL_CONTROLLER_TYPE_XBOXONE:
    type = LI_CTYPE_XBOX;
    break;
  case SDL_CONTROLLER_TYPE_PS3:
  case SDL_CONTROLLER_TYPE_PS4:
  case SDL_CONTROLLER_TYPE_PS5:
    type = LI_CTYPE_PS;
    break;
  case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO:
#if SDL_VERSION_ATLEAST(2, 24, 0)
  case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_LEFT:
  case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT:
  case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_PAIR:
#endif
    type = LI_CTYPE_NINTENDO;
    break;
  }

  LiSendControllerArrivalEvent(state->id, activeGamepadMask, type, supportedButtonFlags, capabilities);
#endif
}

static PGAMEPAD_STATE get_gamepad(SDL_JoystickID sdl_id, bool add) {
  // See if a gamepad already exists
  for (int i = 0;i<MAX_GAMEPADS;i++) {
    if (gamepads[i].initialized && gamepads[i].sdl_id == sdl_id)
      return &gamepads[i];
  }

  if (!add)
    return NULL;

  for (int i = 0;i<MAX_GAMEPADS;i++) {
    if (!gamepads[i].initialized) {
      gamepads[i].sdl_id = sdl_id;
      gamepads[i].id = i;
      gamepads[i].initialized = true;

      activeGamepadMask |= (1 << i);

      return &gamepads[i];
    }
  }

  return &gamepads[0];
}

static void add_gamepad(int joystick_index) {
  SDL_GameController* controller = SDL_GameControllerOpen(joystick_index);
  if (!controller) {
    fprintf(stderr, "Could not open gamecontroller %i: %s\n", joystick_index, SDL_GetError());
    return;
  }

  SDL_Joystick* joystick = SDL_GameControllerGetJoystick(controller);
  SDL_JoystickID joystick_id = SDL_JoystickInstanceID(joystick);

  // Check if we have already set up a state for this gamepad
  PGAMEPAD_STATE state = get_gamepad(joystick_id, false);
  if (state) {
    // This was probably a gamepad added during initialization, so we've already
    // got state set up. However, we still need to inform the host about it, since
    // we couldn't do that during initialization (since we weren't connected yet).
    send_controller_arrival(state);

    SDL_GameControllerClose(controller);
    return;
  }

  // Create a new gamepad state
  state = get_gamepad(joystick_id, true);
  state->controller = controller;

#if !SDL_VERSION_ATLEAST(2, 0, 9)
  state->haptic = SDL_HapticOpenFromJoystick(joystick);
  if (haptic && (SDL_HapticQuery(state->haptic) & SDL_HAPTIC_LEFTRIGHT) == 0) {
    SDL_HapticClose(state->haptic);
    state->haptic = NULL;
  }
  state->haptic_effect_id = -1;
#endif

  // Send the controller arrival event to the host
  send_controller_arrival(state);

  sdl_gamepads++;
}

static void remove_gamepad(SDL_JoystickID sdl_id) {
  for (int i = 0;i<MAX_GAMEPADS;i++) {
    if (gamepads[i].initialized && gamepads[i].sdl_id == sdl_id) {
#if !SDL_VERSION_ATLEAST(2, 0, 9)
      if (gamepads[i].haptic_effect_id >= 0) {
        SDL_HapticDestroyEffect(gamepads[i].haptic, gamepads[i].haptic_effect_id);
      }

      if (gamepads[i].haptic) {
        SDL_HapticClose(gamepads[i].haptic);
      }
#endif

      SDL_GameControllerClose(gamepads[i].controller);

      // This will cause disconnection of the virtual controller on the host PC
      activeGamepadMask &= ~(1 << i);
      LiSendMultiControllerEvent(i, activeGamepadMask, 0, 0, 0, 0, 0, 0, 0);

      memset(&gamepads[i], 0, sizeof(*gamepads));
      sdl_gamepads--;
      break;
    }
  }
}

void sdlinput_init(char* mappings) {
  memset(gamepads, 0, sizeof(gamepads));

  SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
#if !SDL_VERSION_ATLEAST(2, 0, 9)
  SDL_InitSubSystem(SDL_INIT_HAPTIC);
#endif
  SDL_GameControllerAddMappingsFromFile(mappings);

  // Add game controllers here to ensure an accurate count
  // goes to the host when starting a new session.
  for (int i = 0; i < SDL_NumJoysticks(); ++i) {
    if (SDL_IsGameController(i))
      add_gamepad(i);
  }
}

int sdlinput_handle_event(SDL_Window* window, SDL_Event* event) {
  int button = 0;
  unsigned char touchEventType;
  PGAMEPAD_STATE gamepad;
  switch (event->type) {
  case SDL_MOUSEMOTION:
    if (SDL_GetRelativeMouseMode())
      LiSendMouseMoveEvent(event->motion.xrel, event->motion.yrel);
    else {
      int w, h;
      SDL_GetWindowSize(window, &w, &h);
      LiSendMousePositionEvent(event->motion.x, event->motion.y, w, h);
    }
    break;
  case SDL_MOUSEWHEEL:
#if SDL_VERSION_ATLEAST(2, 0, 18)
    LiSendHighResHScrollEvent((short)(event->wheel.preciseX * 120)); // WHEEL_DELTA
    LiSendHighResScrollEvent((short)(event->wheel.preciseY * 120)); // WHEEL_DELTA
#else
    LiSendHScrollEvent(event->wheel.x);
    LiSendScrollEvent(event->wheel.y);
#endif
    break;
  case SDL_MOUSEBUTTONUP:
  case SDL_MOUSEBUTTONDOWN:
    switch (event->button.button) {
    case SDL_BUTTON_LEFT:
      button = BUTTON_LEFT;
      break;
    case SDL_BUTTON_MIDDLE:
      button = BUTTON_MIDDLE;
      break;
    case SDL_BUTTON_RIGHT:
      button = BUTTON_RIGHT;
      break;
    case SDL_BUTTON_X1:
      button = BUTTON_X1;
      break;
    case SDL_BUTTON_X2:
      button = BUTTON_X2;
      break;
    }

    if (button != 0)
      LiSendMouseButtonEvent(event->type==SDL_MOUSEBUTTONDOWN?BUTTON_ACTION_PRESS:BUTTON_ACTION_RELEASE, button);

    return 0;
  case SDL_KEYDOWN:
  case SDL_KEYUP:
    button = event->key.keysym.sym;
    if (button >= 0x21 && button <= 0x2f)
      button = keyCodes1[button - 0x21];
    else if (button >= 0x3a && button <= 0x40)
      button = keyCodes2[button - 0x3a];
    else if (button >= 0x5b && button <= 0x60)
      button = keyCodes3[button - 0x5b];
    else if (button >= 0x40000039 && button < 0x40000039 + sizeof(keyCodes4))
      button = keyCodes4[button - 0x40000039];
    else if (button >= 0x400000E0 && button <= 0x400000E7)
      button = keyCodes5[button - 0x400000E0];
    else if (button >= 0x61 && button <= 0x7a)
      button -= 0x20;
    else if (button == 0x7f)
      button = 0x2e;

    int modifier = 0;
    switch (event->key.keysym.sym) {
    case SDLK_RSHIFT:
    case SDLK_LSHIFT:
      modifier = MODIFIER_SHIFT;
      break;
    case SDLK_RALT:
    case SDLK_LALT:
      modifier = MODIFIER_ALT;
      break;
    case SDLK_RCTRL:
    case SDLK_LCTRL:
      modifier = MODIFIER_CTRL;
      break;
    case SDLK_RGUI:
    case SDLK_LGUI:
      modifier = MODIFIER_META;
      break;
    }

    if (modifier != 0) {
      if (event->type==SDL_KEYDOWN)
        keyboard_modifiers |= modifier;
      else
        keyboard_modifiers &= ~modifier;
    }

    LiSendKeyboardEvent(0x80 << 8 | button, event->type==SDL_KEYDOWN?KEY_ACTION_DOWN:KEY_ACTION_UP, keyboard_modifiers);

    // Quit the stream if all the required quit keys are down
    if ((keyboard_modifiers & ACTION_MODIFIERS) == ACTION_MODIFIERS && event->key.keysym.sym == QUIT_KEY && event->type==SDL_KEYUP)
      return SDL_QUIT_APPLICATION;
    else if ((keyboard_modifiers & ACTION_MODIFIERS) == ACTION_MODIFIERS && event->key.keysym.sym == FULLSCREEN_KEY && event->type==SDL_KEYUP)
      return SDL_TOGGLE_FULLSCREEN;
    else if ((keyboard_modifiers & ACTION_MODIFIERS) == ACTION_MODIFIERS && event->key.keysym.sym == UNGRAB_KEY && event->type==SDL_KEYUP)
      return SDL_GetRelativeMouseMode() ? SDL_MOUSE_UNGRAB : SDL_MOUSE_GRAB;
    break;
  case SDL_FINGERDOWN:
  case SDL_FINGERMOTION:
  case SDL_FINGERUP:
    switch (event->type) {
    case SDL_FINGERDOWN:
        touchEventType = LI_TOUCH_EVENT_DOWN;
        break;
    case SDL_FINGERMOTION:
        touchEventType = LI_TOUCH_EVENT_MOVE;
        break;
    case SDL_FINGERUP:
        touchEventType = LI_TOUCH_EVENT_UP;
        break;
    default:
        return SDL_NOTHING;
    }

    // These are already window-relative normalized coordinates, so we just need to clamp them
    event->tfinger.x = SDL_max(SDL_min(1.0f, event->tfinger.x), 0.0f);
    event->tfinger.y = SDL_max(SDL_min(1.0f, event->tfinger.y), 0.0f);

    LiSendTouchEvent(touchEventType, event->tfinger.fingerId, event->tfinger.x, event->tfinger.y,
                     event->tfinger.pressure, 0.0f, 0.0f, LI_ROT_UNKNOWN);
    break;
  case SDL_CONTROLLERAXISMOTION:
    gamepad = get_gamepad(event->caxis.which, false);
    if (!gamepad)
      return SDL_NOTHING;
    switch (event->caxis.axis) {
    case SDL_CONTROLLER_AXIS_LEFTX:
      gamepad->leftStickX = event->caxis.value;
      break;
    case SDL_CONTROLLER_AXIS_LEFTY:
      gamepad->leftStickY = -SDL_max(event->caxis.value, (short)-32767);
      break;
    case SDL_CONTROLLER_AXIS_RIGHTX:
      gamepad->rightStickX = event->caxis.value;
      break;
    case SDL_CONTROLLER_AXIS_RIGHTY:
      gamepad->rightStickY = -SDL_max(event->caxis.value, (short)-32767);
      break;
    case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
      gamepad->leftTrigger = (unsigned char)(event->caxis.value * 255UL / 32767);
      break;
    case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
      gamepad->rightTrigger = (unsigned char)(event->caxis.value * 255UL / 32767);
      break;
    default:
      return SDL_NOTHING;
    }
    LiSendMultiControllerEvent(gamepad->id, activeGamepadMask, gamepad->buttons, gamepad->leftTrigger, gamepad->rightTrigger, gamepad->leftStickX, gamepad->leftStickY, gamepad->rightStickX, gamepad->rightStickY);
    break;
  case SDL_CONTROLLERBUTTONDOWN:
  case SDL_CONTROLLERBUTTONUP:
    gamepad = get_gamepad(event->cbutton.which, false);
    if (!gamepad)
      return SDL_NOTHING;
    if (event->cbutton.button >= SDL_arraysize(SDL_TO_LI_BUTTON_MAP))
      return SDL_NOTHING;

    if (event->type == SDL_CONTROLLERBUTTONDOWN)
      gamepad->buttons |= SDL_TO_LI_BUTTON_MAP[event->cbutton.button];
    else
      gamepad->buttons &= ~SDL_TO_LI_BUTTON_MAP[event->cbutton.button];

    if ((gamepad->buttons & QUIT_BUTTONS) == QUIT_BUTTONS)
      return SDL_QUIT_APPLICATION;

    LiSendMultiControllerEvent(gamepad->id, activeGamepadMask, gamepad->buttons, gamepad->leftTrigger, gamepad->rightTrigger, gamepad->leftStickX, gamepad->leftStickY, gamepad->rightStickX, gamepad->rightStickY);
    break;
  case SDL_CONTROLLERDEVICEADDED:
    add_gamepad(event->cdevice.which);
    break;
  case SDL_CONTROLLERDEVICEREMOVED:
    remove_gamepad(event->cdevice.which);
    break;
#if SDL_VERSION_ATLEAST(2, 0, 14)
  case SDL_CONTROLLERSENSORUPDATE:
    gamepad = get_gamepad(event->csensor.which, false);
    if (!gamepad)
      return SDL_NOTHING;
    switch (event->csensor.sensor) {
    case SDL_SENSOR_ACCEL:
      LiSendControllerMotionEvent(gamepad->id, LI_MOTION_TYPE_ACCEL, event->csensor.data[0], event->csensor.data[1], event->csensor.data[2]);
      break;
    case SDL_SENSOR_GYRO:
      // Convert rad/s to deg/s
      LiSendControllerMotionEvent(gamepad->id, LI_MOTION_TYPE_GYRO,
                                  event->csensor.data[0] * 57.2957795f,
                                  event->csensor.data[1] * 57.2957795f,
                                  event->csensor.data[2] * 57.2957795f);
      break;
    }
    break;
  case SDL_CONTROLLERTOUCHPADDOWN:
  case SDL_CONTROLLERTOUCHPADUP:
  case SDL_CONTROLLERTOUCHPADMOTION:
    gamepad = get_gamepad(event->ctouchpad.which, false);
    if (!gamepad)
      return SDL_NOTHING;
    switch (event->type) {
    case SDL_CONTROLLERTOUCHPADDOWN:
      touchEventType = LI_TOUCH_EVENT_DOWN;
      break;
    case SDL_CONTROLLERTOUCHPADUP:
      touchEventType = LI_TOUCH_EVENT_UP;
      break;
    case SDL_CONTROLLERTOUCHPADMOTION:
      touchEventType = LI_TOUCH_EVENT_MOVE;
      break;
    default:
      return SDL_NOTHING;
    }
    LiSendControllerTouchEvent(gamepad->id, touchEventType, event->ctouchpad.finger,
                               event->ctouchpad.x, event->ctouchpad.y, event->ctouchpad.pressure);
    break;
#endif
  }

  return SDL_NOTHING;
}

void sdlinput_rumble(unsigned short controller_id, unsigned short low_freq_motor, unsigned short high_freq_motor) {
  if (controller_id >= MAX_GAMEPADS)
    return;

  PGAMEPAD_STATE state = &gamepads[controller_id];

  if (!state->initialized)
    return;

#if SDL_VERSION_ATLEAST(2, 0, 9)
  SDL_GameControllerRumble(state->controller, low_freq_motor, high_freq_motor, 30000);
#else
  SDL_Haptic* haptic = state->haptic;
  if (!haptic)
    return;

  if (state->haptic_effect_id >= 0)
    SDL_HapticDestroyEffect(haptic, state->haptic_effect_id);

  if (low_freq_motor == 0 && high_freq_motor == 0)
    return;

  SDL_HapticEffect effect;
  SDL_memset(&effect, 0, sizeof(effect));
  effect.type = SDL_HAPTIC_LEFTRIGHT;
  effect.leftright.length = SDL_HAPTIC_INFINITY;

  // SDL haptics range from 0-32767 but XInput uses 0-65535, so divide by 2 to correct for SDL's scaling
  effect.leftright.large_magnitude = low_freq_motor / 2;
  effect.leftright.small_magnitude = high_freq_motor / 2;

  state->haptic_effect_id = SDL_HapticNewEffect(haptic, &effect);
  if (state->haptic_effect_id >= 0)
    SDL_HapticRunEffect(haptic, state->haptic_effect_id, 1);
#endif
}

void sdlinput_rumble_triggers(unsigned short controller_id, unsigned short left_trigger, unsigned short right_trigger) {
  PGAMEPAD_STATE state = &gamepads[controller_id];

  if (!state->initialized)
    return;

#if SDL_VERSION_ATLEAST(2, 0, 14)
  SDL_GameControllerRumbleTriggers(state->controller, left_trigger, right_trigger, 30000);
#endif
}

void sdlinput_set_motion_event_state(unsigned short controller_id, unsigned char motion_type, unsigned short report_rate_hz) {
  PGAMEPAD_STATE state = &gamepads[controller_id];

  if (!state->initialized)
    return;

#if SDL_VERSION_ATLEAST(2, 0, 14)
  switch (motion_type) {
  case LI_MOTION_TYPE_ACCEL:
    SDL_GameControllerSetSensorEnabled(state->controller, SDL_SENSOR_ACCEL, report_rate_hz ? SDL_TRUE : SDL_FALSE);
    break;
  case LI_MOTION_TYPE_GYRO:
    SDL_GameControllerSetSensorEnabled(state->controller, SDL_SENSOR_GYRO, report_rate_hz ? SDL_TRUE : SDL_FALSE);
    break;
  }
#endif
}

void sdlinput_set_controller_led(unsigned short controller_id, unsigned char r, unsigned char g, unsigned char b) {
  PGAMEPAD_STATE state = &gamepads[controller_id];

  if (!state->initialized)
    return;

#if SDL_VERSION_ATLEAST(2, 0, 14)
  SDL_GameControllerSetLED(state->controller, r, g, b);
#endif
}