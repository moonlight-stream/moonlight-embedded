package com.limelight.input.gamepad;

import com.limelight.input.Device;
import com.limelight.input.DeviceListener;
import com.limelight.input.gamepad.GamepadMapping.Mapping;
import com.limelight.input.gamepad.SourceComponent.Type;
import com.limelight.nvstream.NvConnection;
import com.limelight.nvstream.input.ControllerPacket;
import com.limelight.settings.GamepadSettingsManager;

/**
 * Represents a gamepad connected to the system
 * @author Diego Waxemberg
 */
public class Gamepad implements DeviceListener {
	
	private short inputMap = 0x0000;
	private byte leftTrigger = 0x00;
	private byte rightTrigger = 0x00;
	private short rightStickX = 0x0000;
	private short rightStickY = 0x0000;
	private short leftStickX = 0x0000;
	private short leftStickY = 0x0000;

	private NvConnection conn;
	
	public Gamepad(NvConnection conn) {
		this.conn = conn;
	}
	
	public Gamepad() {
		this(null);
	}

	public void setConnection(NvConnection conn) {
		this.conn = conn;
	}
	
	@Override
	public void handleButton(Device device, int buttonId, boolean pressed) {
		GamepadMapping mapping = GamepadSettingsManager.getSettings();
		
		Mapping mapped = mapping.get(new SourceComponent(Type.BUTTON, buttonId));
		if (mapped == null) {
			System.out.println("Unmapped button pressed: " + buttonId);
			return;
		}
		
		if (!mapped.contComp.isAnalog()) {
			handleDigitalComponent(mapped, pressed);
		} else {
			handleAnalogComponent(mapped.contComp, sanitizeValue(mapped, pressed));
		}
		
		//printInfo(device, new SourceComponent(Type.BUTTON, buttonId), mapped.contComp, pressed ? 1F : 0F);
	}
	
	@Override
	public void handleAxis(Device device, int axisId, float newValue, float lastValue) {
		GamepadMapping mapping = GamepadSettingsManager.getSettings();
		
		Mapping mapped = mapping.get(new SourceComponent(Type.AXIS, axisId));
		if (mapped == null) {
			System.out.println("Unmapped axis moved: " + axisId);
			return;
		}
		
		float value =  sanitizeValue(mapped, newValue);
		
		if (mapped.contComp.isAnalog()) {
			handleAnalogComponent(mapped.contComp, value);
		} else {
			handleDigitalComponent(mapped, (value > 0.5));
		}
		
		//printInfo(device, new SourceComponent(Type.AXIS, axisId), mapped.contComp, newValue);
	}
	
	private float sanitizeValue(Mapping mapped, boolean value) {
		if (mapped.invert) {
			return value ? 0F : 1F;
		} else {
			return value ? 1F : 0F;
		}
	}

	private float sanitizeValue(Mapping mapped, float value) {
		if (mapped.invert) {
			return -value;
		} else {
			return value;
		}
	}

	private void handleAnalogComponent(GamepadComponent padComp, float value) {
		switch (padComp) {
		case LS_X:
			leftStickX = (short)Math.round(value * 0x7FFF);
			break;
		case LS_Y:
			leftStickY = (short)Math.round(value * 0x7FFF);
			break;
		case RS_X:
			rightStickX = (short)Math.round(value * 0x7FFF);
			break;
		case RS_Y:
			rightStickY = (short)Math.round(value * 0x7FFF);
			break;
		case LT:
			leftTrigger = (byte)Math.round(value * 0xFF);
			break;
		case RT:
			rightTrigger = (byte)Math.round(value * 0xFF);
			break;
		default:
			System.out.println("A mapping error has occured. Ignoring: " + padComp.name());
			break;
		}
		if (conn != null) {
			sendControllerPacket();
		}
	}
	
	
	private void handleDigitalComponent(Mapping mapped, boolean pressed) {
		switch (mapped.contComp) {
		case BTN_A:
			toggle(ControllerPacket.A_FLAG, pressed);
			break;
		case BTN_X:
			toggle(ControllerPacket.X_FLAG, pressed);
			break;
		case BTN_Y:
			toggle(ControllerPacket.Y_FLAG, pressed);
			break;
		case BTN_B:
			toggle(ControllerPacket.B_FLAG, pressed);
			break;
		case DPAD_UP:
			toggle(ControllerPacket.UP_FLAG, pressed);
			break;
		case DPAD_DOWN:
			toggle(ControllerPacket.DOWN_FLAG, pressed);
			break;
		case DPAD_LEFT:
			toggle(ControllerPacket.LEFT_FLAG, pressed);
			break;
		case DPAD_RIGHT:
			toggle(ControllerPacket.RIGHT_FLAG, pressed);
			break;
		case LS_THUMB:
			toggle(ControllerPacket.LS_CLK_FLAG, pressed);
			break;
		case RS_THUMB:
			toggle(ControllerPacket.RS_CLK_FLAG, pressed);
			break;
		case LB:
			toggle(ControllerPacket.LB_FLAG, pressed);
			break;
		case RB:
			toggle(ControllerPacket.RB_FLAG, pressed);
			break;
		case BTN_START:
			toggle(ControllerPacket.PLAY_FLAG, pressed);
			break;
		case BTN_BACK:
			toggle(ControllerPacket.BACK_FLAG, pressed);
			break;
		case BTN_SPECIAL:
			toggle(ControllerPacket.SPECIAL_BUTTON_FLAG, pressed);
			break;
		default:
			System.out.println("A mapping error has occured. Ignoring: " + mapped.contComp.name());
			return;
		}
		if (conn != null) {
			sendControllerPacket();
		}
	}
	
	/*
	 * Sends a controller packet to the specified connection containing the current gamepad values
	 */
	private void sendControllerPacket() {
		if (conn != null) {
			conn.sendControllerInput(inputMap, leftTrigger, rightTrigger, 
					leftStickX, leftStickY, rightStickX, rightStickY);
		}
	}

	/*
	 * Prints out the specified event information for the given gamepad
	 * used for debugging, normally unused.
	 */
	@SuppressWarnings("unused")
	private void printInfo(Device device, SourceComponent sourceComp, GamepadComponent padComp, float value) {

		StringBuilder builder = new StringBuilder();
		
		builder.append(sourceComp.getType().name() + ": ");
		builder.append(sourceComp.getId() + " ");
		builder.append("mapped to: " + padComp + " ");
		builder.append("changed to ");

		System.out.println(builder.toString());
	}

	/*
	 * Toggles a flag that indicates the specified button was pressed or released
	 */
	private void toggle(short button, boolean pressed) {
		if (pressed) {
			inputMap |= button;
		} else {
			inputMap &= ~button;
		}
	}
	
}
