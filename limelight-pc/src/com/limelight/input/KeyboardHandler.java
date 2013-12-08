package com.limelight.input;

import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;

import com.limelight.nvstream.NvConnection;
import com.limelight.nvstream.input.KeyboardPacket;

public class KeyboardHandler implements KeyListener {
	
	private static KeyboardTranslator translator;
	
	public KeyboardHandler(NvConnection conn) {
		translator = new KeyboardTranslator(conn);
	}
	
	@Override
	public void keyPressed(KeyEvent event) {
		short keyMap = translator.translate(event.getKeyCode());
		
		byte modifier = 0x0;
		
		int modifiers = event.getModifiersEx();
		if ((modifiers & KeyEvent.SHIFT_DOWN_MASK) != 0) {
			modifier |= KeyboardPacket.MODIFIER_SHIFT;
		}
		if ((modifiers & KeyEvent.CTRL_DOWN_MASK) != 0) {
			modifier |= KeyboardPacket.MODIFIER_CTRL;
		}
		if ((modifiers & KeyEvent.ALT_DOWN_MASK) != 0) {
			modifier |= KeyboardPacket.MODIFIER_ALT;
		}
		
		translator.sendKeyDown(keyMap, modifier);
	}

	@Override
	public void keyReleased(KeyEvent event) {
		short keyMap = translator.translate(event.getKeyCode());
			
		byte modifier = 0x0;
		
		int modifiers = event.getModifiersEx();
		if ((modifiers & KeyEvent.SHIFT_DOWN_MASK) != 0) {
			modifier |= KeyboardPacket.MODIFIER_SHIFT;
		}
		if ((modifiers & KeyEvent.CTRL_DOWN_MASK) != 0) {
			modifier |= KeyboardPacket.MODIFIER_CTRL;
		}
		if ((modifiers & KeyEvent.ALT_DOWN_MASK) != 0) {
			modifier |= KeyboardPacket.MODIFIER_ALT;
		}
		
		translator.sendKeyUp(keyMap, modifier);
	}

	@Override
	public void keyTyped(KeyEvent event) {
	}

}
