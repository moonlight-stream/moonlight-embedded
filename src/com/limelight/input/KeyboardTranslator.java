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
	 * @returns a GFE keycode for the given keycode
	 */
	@Override
	public short translate(int keycode) {
		// change newline to carriage return
		if (keycode == KeyEvent.VK_ENTER) {
			keycode = 0x0d;
		}
		
		// period maps to delete by default so we remap it
		if (keycode == KeyEvent.VK_PERIOD) {
			keycode = 0xbe;
		}
		
		// Nvidia maps period to delete
		if (keycode == KeyEvent.VK_DELETE) {
			keycode = KeyEvent.VK_PERIOD;
		}
		
		return (short) ((KEY_PREFIX << 8) | keycode);
	}

}
