package com.limelight.input;

import java.awt.event.KeyEvent;

import com.limelight.nvstream.NvConnection;
import com.limelight.nvstream.input.KeycodeTranslator;

/**
 * Class to translate a java key code into the codes GFE is expecting
 * @author Diego Waxemberg
 */
public class KeyboardTranslator extends KeycodeTranslator {
	
	/**
	 * GFE's prefix for every key code
	 */
	public static final short KEY_PREFIX = (short) 0x80;
	
	/**
	 * Constructs a new translator for the specified connection
	 * @param conn the connection to which the translated codes are sent
	 */
	public KeyboardTranslator(NvConnection conn) {
		super(conn);
	}
	
	/**
	 * Translates the given keycode and returns the GFE keycode
	 * @param keycode the code to be translated
	 * @return a GFE keycode for the given keycode
	 */
	@Override
	public short translate(int keycode) {
		
		switch (keycode) {
		case KeyEvent.VK_DELETE:
			keycode = 0x2e;
			break;
		case KeyEvent.VK_MINUS:
			keycode = 0xbd;
			break;
		case KeyEvent.VK_EQUALS:
			keycode = 0xbb;
			break;
		case KeyEvent.VK_OPEN_BRACKET:
			keycode = 0xdb;
			break;
		case KeyEvent.VK_CLOSE_BRACKET:
			keycode = 0xdd;
			break;
		case KeyEvent.VK_BACK_SLASH:
			keycode = 0xdc;
			break;
		case KeyEvent.VK_SEMICOLON:
			keycode = 0xba;
			break;
		case KeyEvent.VK_QUOTE:
			keycode = 0xde;
			break;
		case KeyEvent.VK_ENTER:
			keycode = 0x0d;
			break;
		case KeyEvent.VK_COMMA:
			keycode = 0xbc;
			break;
		case KeyEvent.VK_PERIOD:
			keycode = 0xbe;
			break;
		case KeyEvent.VK_SLASH:
			keycode = 0xbf;
			break;
		}
		
		return (short) ((KEY_PREFIX << 8) | keycode);
	}

}
