/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2015 Iwan Timmer
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

#ifdef HAVE_SDL

#include "sdlinput.h"

#include "limelight-common/Limelight.h"

#define QUIT_MODIFIERS (MODIFIER_SHIFT|MODIFIER_ALT|MODIFIER_CTRL)
#define QUIT_KEY SDLK_q

typedef struct _GAMEPAD_STATE {
  char leftTrigger, rightTrigger;
  short leftStickX, leftStickY;
  short rightStickX, rightStickY;
  int buttons;
  SDL_JoystickID sdl_id;
  short id;
  bool initialized;
} GAMEPAD_STATE, *PGAMEPAD_STATE;

static GAMEPAD_STATE gamepads[4];

static int keyboard_modifiers;

void sdlinput_init() {
  memset(gamepads, 0, sizeof(gamepads));

  SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
  SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");

  for (int i = 0; i < SDL_NumJoysticks(); ++i) {
    if (SDL_IsGameController(i)) {
      if (!SDL_GameControllerOpen(i)) {
        fprintf(stderr, "Could not open gamecontroller %i: %s\n", i, SDL_GetError());
      }
    }
  }
}

static PGAMEPAD_STATE get_gamepad(SDL_JoystickID sdl_id) {
  for (int i = 0;i<4;i++) {
    if (gamepads[i].sdl_id == sdl_id)
      return &gamepads[0];
    else if (!gamepads[i].initialized) {
      gamepads[i].sdl_id = sdl_id;
      gamepads[i].id = i;
      gamepads[i].initialized = true;
      return &gamepads[0];
    }
  }
  return &gamepads[0];
}

bool sdlinput_handle_event(SDL_Event* event) {
  int button = 0;
  PGAMEPAD_STATE gamepad;
  switch (event->type) {
  case SDL_MOUSEMOTION:
    LiSendMouseMoveEvent(event->motion.xrel, event->motion.yrel);
    break;
  case SDL_MOUSEWHEEL:
    LiSendScrollEvent(event->wheel.y);
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
    }

    if (button != 0)
      LiSendMouseButtonEvent(event->type==SDL_MOUSEBUTTONDOWN?BUTTON_ACTION_PRESS:BUTTON_ACTION_RELEASE, button);

    break;
  case SDL_KEYDOWN:
  case SDL_KEYUP:
    button = event->key.keysym.sym;
    if (button >= (0x40000000 + 0x39) && button < (0x40000000 + 0x39 + sizeof(keyCodes)))
      button = keyCodes[button - 0x40000039];
    else if (button >= 0x61)
      button -= 0x20;

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
    }

    if (modifier != 0) {
      if (event->type==SDL_KEYDOWN)
        keyboard_modifiers |= modifier;
      else
        keyboard_modifiers &= ~modifier;
    }

    // Quit the stream if all the required quit keys are down
    if ((keyboard_modifiers & QUIT_MODIFIERS) == QUIT_MODIFIERS && event->key.keysym.sym == QUIT_KEY && event->type==SDL_KEYUP)
      return false;

    LiSendKeyboardEvent(0x80 << 8 | button, event->type==SDL_KEYDOWN?KEY_ACTION_DOWN:KEY_ACTION_UP, keyboard_modifiers);
    break;
  case SDL_CONTROLLERAXISMOTION:
    gamepad = get_gamepad(event->caxis.which);
    switch (event->caxis.axis) {
    case SDL_CONTROLLER_AXIS_LEFTX:
      gamepad->leftStickX = event->caxis.value;
      break;
    case SDL_CONTROLLER_AXIS_LEFTY:
      gamepad->leftStickY = -event->caxis.value - 1;
      break;
    case SDL_CONTROLLER_AXIS_RIGHTX:
      gamepad->rightStickX = event->caxis.value;
      break;
    case SDL_CONTROLLER_AXIS_RIGHTY:
      gamepad->rightStickY = -event->caxis.value - 1;
      break;
    case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
      gamepad->leftTrigger = (event->caxis.value >> 8) + 127;
      break;
    case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
      gamepad->rightTrigger = (event->caxis.value >> 8) + 127;
      break;
    default:
      return true;
    }
    LiSendMultiControllerEvent(gamepad->id, gamepad->buttons, gamepad->leftTrigger, gamepad->rightTrigger, gamepad->leftStickX, gamepad->leftStickY, gamepad->rightStickX, gamepad->rightStickY);
    break;
  case SDL_CONTROLLERBUTTONDOWN:
  case SDL_CONTROLLERBUTTONUP:
    gamepad = get_gamepad(event->cbutton.which);
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
      return true;
    }
    if (event->type == SDL_CONTROLLERBUTTONDOWN)
      gamepad->buttons |= button;
    else
      gamepad->buttons &= ~button;

    LiSendMultiControllerEvent(gamepad->id, gamepad->buttons, gamepad->leftTrigger, gamepad->rightTrigger, gamepad->leftStickX, gamepad->leftStickY, gamepad->rightStickX, gamepad->rightStickY);
    break;
  }
  return true;
}

#endif /* HAVE_SDL */
