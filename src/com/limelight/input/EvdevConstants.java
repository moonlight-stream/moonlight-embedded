package com.limelight.input;

import java.awt.event.KeyEvent;

/**
 *
 * @author iwan
 */
public class EvdevConstants {
	
	public static final int MAX_STRUCT_SIZE_BYTES = 24;
	
	public static final short KEY_CODES[] = {
		0, //KeyEvent.VK_RESERVED
		KeyEvent.VK_ESCAPE,
		KeyEvent.VK_1,
		KeyEvent.VK_2,
		KeyEvent.VK_3,
		KeyEvent.VK_4,
		KeyEvent.VK_5,
		KeyEvent.VK_6,
		KeyEvent.VK_7,
		KeyEvent.VK_8,
		KeyEvent.VK_9,
		KeyEvent.VK_0,
		KeyEvent.VK_MINUS,
		KeyEvent.VK_EQUALS,
		KeyEvent.VK_BACK_SPACE,
		KeyEvent.VK_TAB,
		KeyEvent.VK_Q,
		KeyEvent.VK_W,
		KeyEvent.VK_E,
		KeyEvent.VK_R,
		KeyEvent.VK_T,
		KeyEvent.VK_Y,
		KeyEvent.VK_U,
		KeyEvent.VK_I,
		KeyEvent.VK_O,
		KeyEvent.VK_P,
		KeyEvent.VK_BRACELEFT,
		KeyEvent.VK_BRACERIGHT,
		KeyEvent.VK_ENTER,
		KeyEvent.VK_CONTROL, // Left control */
		KeyEvent.VK_A,
		KeyEvent.VK_S,
		KeyEvent.VK_D,
		KeyEvent.VK_F,
		KeyEvent.VK_G,
		KeyEvent.VK_H,
		KeyEvent.VK_J,
		KeyEvent.VK_K,
		KeyEvent.VK_L,
		KeyEvent.VK_SEMICOLON,
		0, //KeyEvent.VK_APOSTROPHE,
		0, //KeyEvent.VK_GRAVE,
		KeyEvent.VK_SHIFT, // Left shift
		KeyEvent.VK_BACK_SLASH,
		KeyEvent.VK_Z,
		KeyEvent.VK_X,
		KeyEvent.VK_C,
		KeyEvent.VK_V,
		KeyEvent.VK_B,
		KeyEvent.VK_N,
		KeyEvent.VK_M,
		KeyEvent.VK_COMMA,
		0, //KeyEvent.VK_DOT,
		KeyEvent.VK_SLASH,
		KeyEvent.VK_SHIFT, // Right shift
		0, //KeyEvent.VK_KPASTERISK,
		KeyEvent.VK_ALT, // Left alt
		KeyEvent.VK_SPACE,
		KeyEvent.VK_CAPS_LOCK,
		KeyEvent.VK_F1,
		KeyEvent.VK_F2,
		KeyEvent.VK_F3,
		KeyEvent.VK_F4,
		KeyEvent.VK_F5,
		KeyEvent.VK_F6,
		KeyEvent.VK_F7,
		KeyEvent.VK_F8,
		KeyEvent.VK_F9,
		KeyEvent.VK_F10,
		KeyEvent.VK_NUM_LOCK,
		KeyEvent.VK_SCROLL_LOCK,
		KeyEvent.VK_NUMPAD7,
		KeyEvent.VK_NUMPAD8,
		KeyEvent.VK_NUMPAD9,
		0, //KeyEvent.VK_NUMPAD_MINUS,
		KeyEvent.VK_NUMPAD4,
		KeyEvent.VK_NUMPAD5,
		KeyEvent.VK_NUMPAD6,
		0, //KeyEvent.VK_NUMPADPLUS,
		KeyEvent.VK_NUMPAD1,
		KeyEvent.VK_NUMPAD2,
		KeyEvent.VK_NUMPAD3,
		KeyEvent.VK_NUMPAD0,
		0, //KeyEvent.VK_NUMPADDOT,
		0,
		0, //KeyEvent.VK_ZENKAKUHANKAKU,
		0, //KeyEvent.VK_102ND,
		KeyEvent.VK_F11,
		KeyEvent.VK_F12,
		0, //KeyEvent.VK_RO,
		KeyEvent.VK_KATAKANA,
		KeyEvent.VK_HIRAGANA,
		0, //KeyEvent.VK_HENKAN,
		0, //KeyEvent.VK_KATAKANAHIRAGANA,
		0, //KeyEvent.VK_MUHENKAN,
		0, //KeyEvent.VK_KPJPCOMMA,
		0, //KeyEvent.VK_KPENTER,
		KeyEvent.VK_CONTROL, // Right ctrl
		0, //KeyEvent.VK_KPSLASH,
		0, //KeyEvent.VK_SYSRQ,
		KeyEvent.VK_ALT, // Right alt
		0, //KeyEvent.VK_LINEFEED,
		KeyEvent.VK_HOME,
		KeyEvent.VK_UP,
		KeyEvent.VK_PAGE_UP,
		KeyEvent.VK_LEFT,
		KeyEvent.VK_RIGHT,
		KeyEvent.VK_END,
		KeyEvent.VK_DOWN,
		KeyEvent.VK_PAGE_DOWN,
		KeyEvent.VK_INSERT,
		KeyEvent.VK_DELETE,
		0, //KeyEvent.VK_MACRO,
		0, //KeyEvent.VK_MUTE,
		0, //KeyEvent.VK_VOLUMEDOWN,
		0, //KeyEvent.VK_VOLUMEUP,
		0, //KeyEvent.VK_POWER, /* SC System Power Down */
		0, //KeyEvent.VK_KPEQUAL,
		0, //KeyEvent.VK_KPPLUSMINUS,
		KeyEvent.VK_PAUSE,
		0, //KeyEvent.VK_SCALE, /* AL Compiz Scale (Expose) */
	};
	
	/* Mouse constants */
	public static final short REL_X = 0x00;
	public static final short REL_Y = 0x01;
	public static final short REL_WHEEL = 0x08;
	
	public static final short BTN_LEFT = 0x110;
	public static final short BTN_RIGHT = 0x111;
	public static final short BTN_MIDDLE = 0x112;
	
	/* Gamepad constants */
	public static final short ABS_X = 0x00;
	public static final short ABS_Y = 0x01;
	public static final short ABS_Z = 0x02;
	public static final short ABS_RX = 0x03;
	public static final short ABS_RY = 0x04;
	public static final short ABS_RZ = 0x05;
	
	public static final short ABS_HAT0X = 0x10;
	public static final short ABS_HAT0Y = 0x11;
	
	public static final short BTN_SOUTH = 0x130;
	public static final short BTN_EAST = 0x131;
	public static final short BTN_NORTH = 0x133;
	public static final short BTN_WEST = 0x134;
	
	public static final short BTN_SELECT = 0x13a;
	public static final short BTN_START = 0x13b;
	public static final short BTN_MODE = 0x13c;
	public static final short BTN_THUMBL = 0x13d;
	public static final short BTN_THUMBR = 0x13e;
	public static final short BTN_TL = 0x136;
	public static final short BTN_TR = 0x137;
	public static final short BTN_TL2 = 0x138;
	public static final short BTN_TR2 = 0x139;
	
	public static final short BTN_DPAD_UP = 0x220;
	public static final short BTN_DPAD_DOWN = 0x221;
	public static final short BTN_DPAD_LEFT = 0x222;
	public static final short BTN_DPAD_RIGHT = 0x223;

	/* Event types */
	public static final short EV_KEY = 0x01;
	public static final short EV_REL = 0x02;
	public static final short EV_ABS = 0x03;
	
	/* Events */
	public static final int KEY_RELEASED = 0;
	public static final int KEY_PRESSED = 1;
}
