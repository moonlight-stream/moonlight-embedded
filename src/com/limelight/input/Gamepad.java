package com.limelight.input;

import com.limelight.nvstream.NvConnection;
import com.limelight.nvstream.input.ControllerPacket;

import net.java.games.input.Component;
import net.java.games.input.Controller;
import net.java.games.input.Event;
import net.java.games.input.EventQueue;

public class Gamepad {
	private Controller pad;
	private GamepadMapping config;

	private short inputMap = 0x0000;
	private byte leftTrigger = 0x00;
	private byte rightTrigger = 0x00;
	private short rightStickX = 0x0000;
	private short rightStickY = 0x0000;
	private short leftStickX = 0x0000;
	private short leftStickY = 0x0000;

	public Gamepad(Controller pad, GamepadMapping settings) {
		this.config = settings;
		this.pad = pad;

		for (Component comp : pad.getComponents()) {
			initValue(comp);
		}
	}

	public GamepadMapping getConfiguration() {
		return config;
	}

	private void initValue(Component comp) {
		handleComponent(comp, comp.getPollData());
	}

	public boolean poll() {
		return pad.poll();
	}

	private void sendControllerPacket(NvConnection conn) {
		if (conn != null) {
			conn.sendControllerInput(inputMap, leftTrigger, rightTrigger, 
					leftStickX, leftStickY, rightStickX, rightStickY);
		}
	}

	public EventQueue getEvents() {
		return pad.getEventQueue();
	}

	public void handleEvents(NvConnection conn) {
		EventQueue queue = pad.getEventQueue();
		Event event = new Event();

		while(queue.getNextEvent(event)) {

			/* uncommented for debugging */
			//printInfo(pad, event);

			handleEvent(event);

			sendControllerPacket(conn);
		}

	}

	/*
	 * used for debugging, normally unused.
	 */
	@SuppressWarnings("unused")
	private void printInfo(Controller gamepad, Event event) {
		Component comp = event.getComponent();

		StringBuilder builder = new StringBuilder(gamepad.getName());

		builder.append(" at ");
		builder.append(event.getNanos()).append(": ");
		builder.append(comp.getName()).append(" changed to ");


		float value = event.getValue(); 
		if(comp.isAnalog()) {
			builder.append(value);
		} else {
			if(value==1.0f) {
				builder.append("On");
			} else {
				builder.append("Off");
			}
		}

		System.out.println(builder.toString());
	}

	private void handleEvent(Event event) {
		Component comp = event.getComponent();
		float value = event.getValue();

		handleComponent(comp, value);
	}

	private void handleComponent(Component comp, float value) {
		ControllerComponent contComp = config.getControllerComponent(comp);
		if (contComp != null) {
			if (contComp.isAnalog()) {
				handleAnalog(contComp, sanitizeValue(contComp, value));
			} else {
				handleButtons(contComp, sanitizeValue(contComp, value));
			}
		}
	}

	private float sanitizeValue(ControllerComponent contComp, float value) {
		float sanitized = value;
		if (contComp.invert()) {
			sanitized = -sanitized;
		}
		if (contComp.isTrigger()) {
			sanitized = (sanitized + 1)/2;
		}
		return sanitized;
	}
	
	private void toggle(short button, boolean press) {
		if (press) {
			inputMap |= button;
		} else {
			inputMap &= ~button;
		}
	}

	private void handleAnalog(ControllerComponent contComp, float value) {
		switch (contComp) {
		case LS_X:
			leftStickX = (short)Math.round(value * 0x7FFF);
			break;
		case LS_Y:
			leftStickY = (short)Math.round(value * 0x7FFF);
			break;
		case RS_X:
			leftStickX = (short)Math.round(value * 0x7FFF);
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
			System.out.println("A mapping error has occured. Ignoring: " + contComp.name());
			break;
		}
	}

	private void handleButtons(ControllerComponent contComp, float value) {
		boolean press = false;

		if (value > 0.5F) {
			press = true;
		}

		switch (contComp) {
		case BTN_A:
			toggle(ControllerPacket.A_FLAG, press);
			break;
		case BTN_X:
			toggle(ControllerPacket.X_FLAG, press);
			break;
		case BTN_Y:
			toggle(ControllerPacket.Y_FLAG, press);
			break;
		case BTN_B:
			toggle(ControllerPacket.B_FLAG, press);
			break;
		case DPAD_UP:
			toggle(ControllerPacket.UP_FLAG, press);
			break;
		case DPAD_DOWN:
			toggle(ControllerPacket.DOWN_FLAG, press);
			break;
		case DPAD_LEFT:
			toggle(ControllerPacket.LEFT_FLAG, press);
			break;
		case DPAD_RIGHT:
			toggle(ControllerPacket.RIGHT_FLAG, press);
			break;
		case LS_THUMB:
			toggle(ControllerPacket.LS_CLK_FLAG, press);
			break;
		case RS_THUMB:
			toggle(ControllerPacket.RS_CLK_FLAG, press);
			break;
		case LB:
			toggle(ControllerPacket.LB_FLAG, press);
			break;
		case RB:
			toggle(ControllerPacket.RB_FLAG, press);
			break;
		case BTN_START:
			toggle(ControllerPacket.PLAY_FLAG, press);
			break;
		case BTN_BACK:
			toggle(ControllerPacket.BACK_FLAG, press);
			break;
		case BTN_SPECIAL:
			toggle(ControllerPacket.SPECIAL_BUTTON_FLAG, press);
			break;
		default:
			System.out.println("A mapping error has occured. Ignoring: " + contComp.name());
			break;
		}
	}
}
