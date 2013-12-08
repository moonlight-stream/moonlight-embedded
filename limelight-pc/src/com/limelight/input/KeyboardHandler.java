package com.limelight.input;

import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;

import com.limelight.nvstream.NvConnection;

public class KeyboardHandler implements KeyListener {
	
	private static KeyboardTranslator translator;
	
	public KeyboardHandler(NvConnection conn) {
		translator = new KeyboardTranslator(conn);
	}
	
	@Override
	public void keyPressed(KeyEvent event) {
		short keyMap = translator.translate(event.getKeyCode());
		translator.sendKeyDown(keyMap);
	}

	@Override
	public void keyReleased(KeyEvent event) {
		short keyMap = translator.translate(event.getKeyCode());
		translator.sendKeyUp(keyMap);
	}

	@Override
	public void keyTyped(KeyEvent event) {
	}

}
