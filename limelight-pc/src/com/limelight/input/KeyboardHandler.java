package com.limelight.input;

import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;

import com.limelight.gui.StreamFrame;
import com.limelight.nvstream.NvConnection;
import com.limelight.nvstream.input.KeyboardPacket;

public class KeyboardHandler implements KeyListener {
	
	private static KeyboardTranslator translator;
	private StreamFrame parent;
	
	public KeyboardHandler(NvConnection conn, StreamFrame parent) {
		translator = new KeyboardTranslator(conn);
		this.parent = parent;
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

		if ((modifiers & KeyEvent.SHIFT_DOWN_MASK) != 0 &&
			(modifiers & KeyEvent.ALT_DOWN_MASK) != 0 &&
			(modifiers & KeyEvent.CTRL_DOWN_MASK) != 0 &&
			event.getKeyCode() == KeyEvent.VK_Q) {
			System.out.println("quitting");
			parent.close();
			System.exit(0);
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
