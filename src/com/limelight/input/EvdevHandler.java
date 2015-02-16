package com.limelight.input;

import com.limelight.nvstream.NvConnection;
import com.limelight.nvstream.input.ControllerPacket;
import com.limelight.nvstream.input.KeyboardPacket;
import com.limelight.nvstream.input.MouseButtonPacket;
import java.awt.event.KeyEvent;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * Class that handles keyboard input using the Evdev interface
 * @author Iwan Timmer
 */
public class EvdevHandler extends EvdevReader {
	
	private static KeyboardTranslator translator;
	
	/* GFE's prefix for every key code */
	public static final short KEY_PREFIX = (short) 0x80;

	/* Gamepad state */
	private short buttonFlags;
	private byte leftTrigger, rightTrigger;
	private short leftStickX, leftStickY, rightStickX, rightStickY;
	private boolean gamepadSynced;
	
	private short mouseDeltaX, mouseDeltaY;
	private byte mouseScroll;
    
    private byte keyModifiers;
	
	private EvdevAbsolute absLX, absLY, absRX, absRY, absLT, absRT, absDX, absDY;
	
	private NvConnection conn;
	private GamepadMapping mapping;
	
	public EvdevHandler(NvConnection conn, String device, GamepadMapping mapping) throws FileNotFoundException, IOException {
		super(device);
		this.conn = conn;
		this.mapping = mapping;
		
		// We want limelight-common to scale the axis values to match Xinput values
		ControllerPacket.enableAxisScaling = true;
		
		absLX = new EvdevAbsolute(device, mapping.abs_x, mapping.reverse_x);
		absLY = new EvdevAbsolute(device, mapping.abs_y, !mapping.reverse_y);
		absRX = new EvdevAbsolute(device, mapping.abs_rx, mapping.reverse_rx);
		absRY = new EvdevAbsolute(device, mapping.abs_ry, !mapping.reverse_ry);
		absLT = new EvdevAbsolute(device, mapping.abs_z, mapping.reverse_z);
		absRT = new EvdevAbsolute(device, mapping.abs_rz, mapping.reverse_rz);
		absDX = new EvdevAbsolute(device, mapping.abs_dpad_x, mapping.reverse_dpad_x);
		absDY = new EvdevAbsolute(device, mapping.abs_dpad_y, mapping.reverse_dpad_y);
		
		translator = new KeyboardTranslator(conn);
		gamepadSynced = true;
	}

	@Override
	protected void parseEvent(ByteBuffer buffer) {
		if (buffer.limit()==EvdevConstants.MAX_STRUCT_SIZE_BYTES) {
			long time_sec = buffer.getLong();
			long time_usec = buffer.getLong();
		} else {
			int time_sec = buffer.getInt();
			int time_usec = buffer.getInt();
		}
		short type = buffer.getShort();
		short code = buffer.getShort();
		int value = buffer.getInt();
		boolean gamepadModified = false;
		
		if (type==EvdevConstants.EV_SYN) {
			if (!gamepadSynced) {
				conn.sendControllerInput(buttonFlags, leftTrigger, rightTrigger, leftStickX, leftStickY, rightStickX, rightStickY);
				gamepadSynced = true;
			}
			if (mouseDeltaX != 0 || mouseDeltaY != 0) {
				conn.sendMouseMove(mouseDeltaX, mouseDeltaY);
				mouseDeltaX = mouseDeltaY = 0;
			}
			if (mouseScroll != 0) {
				conn.sendMouseScroll(mouseScroll);
				mouseScroll = 0;
			}
		} else if (type==EvdevConstants.EV_KEY) {
			if (code<EvdevConstants.KEY_CODES.length) {
				short gfCode = translator.translate(EvdevConstants.KEY_CODES[code]);

				if (value==EvdevConstants.KEY_PRESSED) {
					if (gfCode==KeyEvent.VK_SHIFT)
						keyModifiers |= KeyboardPacket.MODIFIER_SHIFT;
					else if (gfCode==KeyEvent.VK_CONTROL)
						keyModifiers |= KeyboardPacket.MODIFIER_CTRL;
					else if (gfCode==KeyEvent.VK_ALT)
						keyModifiers |= KeyboardPacket.MODIFIER_ALT;
                    
					conn.sendKeyboardInput(gfCode, KeyboardPacket.KEY_DOWN, keyModifiers);
				} else if (value==EvdevConstants.KEY_RELEASED) {
					if (gfCode==KeyEvent.VK_SHIFT)
						keyModifiers &= ~KeyboardPacket.MODIFIER_SHIFT;
					else if (gfCode==KeyEvent.VK_CONTROL)
						keyModifiers &= ~KeyboardPacket.MODIFIER_CTRL;
					else if (gfCode==KeyEvent.VK_ALT)
						keyModifiers &= ~KeyboardPacket.MODIFIER_ALT;
					
					conn.sendKeyboardInput(gfCode, KeyboardPacket.KEY_UP, keyModifiers);
				}
			} else {
				byte mouseButton = 0;
				short gamepadButton = 0;
				
				if (code==EvdevConstants.BTN_LEFT)
					mouseButton = MouseButtonPacket.BUTTON_LEFT;
				else if (code==EvdevConstants.BTN_MIDDLE)
					mouseButton = MouseButtonPacket.BUTTON_MIDDLE;
				else if (code==EvdevConstants.BTN_RIGHT)
					mouseButton = MouseButtonPacket.BUTTON_RIGHT;
				else if (code==mapping.btn_south)
					gamepadButton = ControllerPacket.A_FLAG;
				else if (code==mapping.btn_west)
					gamepadButton = ControllerPacket.X_FLAG;
				else if (code==mapping.btn_north)
					gamepadButton = ControllerPacket.Y_FLAG;
				else if (code==mapping.btn_east)
					gamepadButton = ControllerPacket.B_FLAG;
				else if (code==mapping.btn_dpad_up)
					gamepadButton = ControllerPacket.UP_FLAG;
				else if (code==mapping.btn_dpad_down)
					gamepadButton = ControllerPacket.DOWN_FLAG;
				else if (code==mapping.btn_dpad_left)
					gamepadButton = ControllerPacket.LEFT_FLAG;
				else if (code==mapping.btn_dpad_right)
					gamepadButton = ControllerPacket.RIGHT_FLAG;
				else if (code==mapping.btn_thumbl)
					gamepadButton = ControllerPacket.LS_CLK_FLAG;
				else if (code==mapping.btn_thumbr)
					gamepadButton = ControllerPacket.RS_CLK_FLAG;
				else if (code==mapping.btn_tl)
					gamepadButton = ControllerPacket.LB_FLAG;
				else if (code==mapping.btn_tr)
					gamepadButton = ControllerPacket.RB_FLAG;
				else if (code==mapping.btn_start)
					gamepadButton = ControllerPacket.PLAY_FLAG;
				else if (code==mapping.btn_select)
					gamepadButton = ControllerPacket.BACK_FLAG;
				else if (code==mapping.btn_mode)
					gamepadButton = ControllerPacket.SPECIAL_BUTTON_FLAG;
			
				if (mouseButton>0) {
					if (value==EvdevConstants.KEY_PRESSED)
						conn.sendMouseButtonDown(mouseButton);
					else if (value==EvdevConstants.KEY_RELEASED)
						conn.sendMouseButtonUp(mouseButton);
				} else {
					gamepadModified = true;
					
					if (gamepadButton != 0) {
						if (value==EvdevConstants.KEY_PRESSED)
							buttonFlags |= gamepadButton;
						else  if (value==EvdevConstants.KEY_RELEASED)
							buttonFlags &= ~gamepadButton;
					} else if (code==mapping.btn_tl2)
						leftTrigger = (byte) (value==EvdevConstants.KEY_PRESSED ? -1 : 0);
					else if (code==mapping.btn_tr2)
						rightTrigger = (byte) (value==EvdevConstants.KEY_PRESSED ? -1 : 0);
					else
						gamepadModified = false;
				}
			}
		} else if (type==EvdevConstants.EV_REL) {
			if (code==EvdevConstants.REL_X)
				mouseDeltaX = (short) value;
			else if (code==EvdevConstants.REL_Y)
				mouseDeltaY = (short) value;
			else if (code==EvdevConstants.REL_WHEEL)
				mouseScroll = (byte) value;
		} else if (type==EvdevConstants.EV_ABS) {
			gamepadModified = true;
			
			if (code==mapping.abs_x)
				leftStickX = accountForDeadzone(absLX.getShort(value));
			else if (code==mapping.abs_y)
				leftStickY = accountForDeadzone(absLY.getShort(value));
			else if (code==mapping.abs_rx)
				rightStickX = accountForDeadzone(absRX.getShort(value));
			else if (code==mapping.abs_ry)
				rightStickY = accountForDeadzone(absRY.getShort(value));
			else if (code==mapping.abs_z)
				leftTrigger = absLT.getByte(value);
			else if (code==mapping.abs_rz)
				rightTrigger = absRT.getByte(value);
			else if (code==mapping.abs_dpad_x) {
				int dir = absDX.getDirection(value);
				if (dir==EvdevAbsolute.UP) {
					buttonFlags |= ControllerPacket.RIGHT_FLAG;
					buttonFlags &= ~ControllerPacket.LEFT_FLAG;
				} else if (dir==EvdevAbsolute.NONE) {
					buttonFlags &= ~ControllerPacket.LEFT_FLAG;
					buttonFlags &= ~ControllerPacket.RIGHT_FLAG;
				} else if (dir==EvdevAbsolute.DOWN) {
					buttonFlags |= ControllerPacket.LEFT_FLAG;
					buttonFlags &= ~ControllerPacket.RIGHT_FLAG;
				}
			} else if (code==mapping.abs_dpad_y) {
				int dir = absDY.getDirection(value);
				if (dir==EvdevAbsolute.DOWN) {
					buttonFlags |= ControllerPacket.UP_FLAG;
					buttonFlags &= ~ControllerPacket.DOWN_FLAG;
				} else if (dir==EvdevAbsolute.NONE) {
					buttonFlags &= ~ControllerPacket.DOWN_FLAG;
					buttonFlags &= ~ControllerPacket.UP_FLAG;
				} else if (dir==EvdevAbsolute.UP) {
					buttonFlags |= ControllerPacket.DOWN_FLAG;
					buttonFlags &= ~ControllerPacket.UP_FLAG;
				}
			} else
				gamepadModified = false;
		}
		
		gamepadSynced &= !gamepadModified;
	}

	private short accountForDeadzone(short value) {
		return Math.abs(value) > mapping.abs_deadzone?value:0;
	}
	
}
