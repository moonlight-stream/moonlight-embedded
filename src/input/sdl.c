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

static GAMEPAD_STATE gamepads[4];

static int keyboard_modifiers;
static int activeGamepadMask = 0;

int sdl_gamepads = 0;

static PGAMEPAD_STATE get_gamepad(SDL_JoystickID sdl_id, bool add) {
  // See if a gamepad already exists
  for (int i = 0;i<4;i++) {
    if (gamepads[i].initialized && gamepads[i].sdl_id == sdl_id)
        return &gamepads[i];
  }

  if (!add)
    return NULL;

  for (int i = 0;i<4;i++) {
    if (!gamepads[i].initialized) {
      gamepads[i].sdl_id = sdl_id;
      gamepads[i].id = i;
      gamepads[i].initialized = true;

      // This will cause connection of the virtual controller on the host PC
      activeGamepadMask |= (1 << i);
      LiSendMultiControllerEvent(i, activeGamepadMask, 0, 0, 0, 0, 0, 0, 0);

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

  if (get_gamepad(joystick_id, false)) {
    // Already added
    SDL_GameControllerClose(controller);
    return;
  }

  PGAMEPAD_STATE state = get_gamepad(joystick_id, true);
  state->controller = controller;

#if !SDL_VERSION_ATLEAST(2, 0, 9)
  state->haptic = SDL_HapticOpenFromJoystick(joystick);
  if (haptic && (SDL_HapticQuery(state->haptic) & SDL_HAPTIC_LEFTRIGHT) == 0) {
    SDL_HapticClose(state->haptic);
    state->haptic = NULL;
  }
  state->haptic_effect_id = -1;
#endif

  sdl_gamepads++;
}

static void remove_gamepad(SDL_JoystickID sdl_id) {
  for (int i = 0;i<4;i++) {
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
  SDL_InitSubSystem(SDL_INIT_HAPTIC);
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
    LiSendHighResScrollEvent((short)(event->wheel.preciseY * 120)); // WHEEL_DELTA
#else
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
    switch (event->cbutton.button) {
    case SDL_CONTROLLER_BUTTON_A:
      button = A_FLAG;
      break;
    case SDL_CONTROLLER_BUTTON_B:
      button = B_FLAG;
      break;
    case SDL_CONTROLLER_BUTTON_Y:
      button = Y_FLAG;
      break;
    case SDL_CONTROLLER_BUTTON_X:
      button = X_FLAG;
      break;
    case SDL_CONTROLLER_BUTTON_DPAD_UP:
      button = UP_FLAG;
      break;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
      button = DOWN_FLAG;
      break;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
      button = RIGHT_FLAG;
      break;
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
      button = LEFT_FLAG;
      break;
    case SDL_CONTROLLER_BUTTON_BACK:
      button = BACK_FLAG;
      break;
    case SDL_CONTROLLER_BUTTON_START:
      button = PLAY_FLAG;
      break;
    case SDL_CONTROLLER_BUTTON_GUIDE:
      button = SPECIAL_FLAG;
      break;
    case SDL_CONTROLLER_BUTTON_LEFTSTICK:
      button = LS_CLK_FLAG;
      break;
    case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
      button = RS_CLK_FLAG;
      break;
    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
      button = LB_FLAG;
      break;
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
      button = RB_FLAG;
      break;
    default:
      return SDL_NOTHING;
    }
    if (event->type == SDL_CONTROLLERBUTTONDOWN)
      gamepad->buttons |= button;
    else
      gamepad->buttons &= ~button;

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
  }
  return SDL_NOTHING;
}

void sdlinput_rumble(unsigned short controller_id, unsigned short low_freq_motor, unsigned short high_freq_motor) {
  if (controller_id >= 4)
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
