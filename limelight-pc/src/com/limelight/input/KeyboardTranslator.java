package com.limelight.input;

import java.awt.event.KeyEvent;

import com.limelight.nvstream.NvConnection;
import com.limelight.nvstream.input.KeycodeTranslator;

public class KeyboardTranslator extends KeycodeTranslator {

	public static final short KEYCODE_A = (short) 0x8041;
	
	public KeyboardTranslator(NvConnection conn) {
		super(conn);
	}
	
	@Override
	public short translate(int keycode) {
		if (keycode >= KeyEvent.VK_A && keycode <= KeyEvent.VK_Z) {
			return (short) (KEYCODE_A + (short)(keycode - KeyEvent.VK_A));
		}
		return 0;
	}

}
