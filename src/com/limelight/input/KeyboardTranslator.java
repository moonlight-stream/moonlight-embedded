package com.limelight.input;

import java.awt.event.KeyEvent;

import com.limelight.nvstream.NvConnection;
import com.limelight.nvstream.input.KeycodeTranslator;

public class KeyboardTranslator extends KeycodeTranslator {

	public static final short KEY_PREFIX = (short) 0x80;
	
	public KeyboardTranslator(NvConnection conn) {
		super(conn);
	}
	
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
