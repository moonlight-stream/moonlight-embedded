package com.limelight.input;

import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;

import com.limelight.gui.StreamFrame;
import com.limelight.nvstream.NvConnection;
import com.limelight.nvstream.input.KeyboardPacket;

/**
 * Class that handles keyboard input
 * @author Diego Waxemberg
 */
public class KeyboardHandler implements KeyListener {

	private static KeyboardTranslator translator;
	private StreamFrame parent;

	/**
	 * Constructs a new keyboard listener that will send key events to the specified connection
	 * and belongs to the specified frame
	 * @param conn the connection to send key events to
	 * @param parent the frame that owns this handler
	 */
	public KeyboardHandler(NvConnection conn, StreamFrame parent) {
		translator = new KeyboardTranslator(conn);
		this.parent = parent;
	}

	/**
	 * Invoked when a key is pressed and will send that key-down event to the host
	 * @param event the key-down event
	 */
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
		} else if (
				(modifiers & KeyEvent.SHIFT_DOWN_MASK) != 0 &&
				(modifiers & KeyEvent.ALT_DOWN_MASK) != 0 &&
				(modifiers & KeyEvent.CTRL_DOWN_MASK) != 0) {
			parent.freeMouse();
			return;
		}



		translator.sendKeyDown(keyMap, modifier);
	}

	/**
	 * Invoked when a key is released and will send that key-up event to the host
	 * @param event the key-up event
	 */
	@Override
	public void keyReleased(KeyEvent event) {
		int modifiers = event.getModifiersEx();
		
		if ((modifiers & KeyEvent.SHIFT_DOWN_MASK) != 0 ||
				(modifiers & KeyEvent.ALT_DOWN_MASK) != 0 ||
				(modifiers & KeyEvent.CTRL_DOWN_MASK) != 0) {
			parent.captureMouse();
		}
		
		short keyMap = translator.translate(event.getKeyCode());

		byte modifier = 0x0;

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

	/**
	 * Unimplemented
	 * @param event unused
	 */
	@Override
	public void keyTyped(KeyEvent event) {
	}

}
