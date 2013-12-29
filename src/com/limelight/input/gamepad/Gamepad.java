package com.limelight.input.gamepad;

import com.limelight.input.gamepad.GamepadMapping.Mapping;
import com.limelight.nvstream.NvConnection;
import com.limelight.nvstream.input.ControllerPacket;

import net.java.games.input.Component;
import net.java.games.input.Controller;
import net.java.games.input.Event;
import net.java.games.input.EventQueue;

/**
 * Represents a gamepad connected to the system
 * @author Diego Waxemberg
 */
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

	/**
	 * Constructs a new gamepad from the specified controller that has the given mappings
	 * @param pad the controller to be used as a gamepad
	 * @param settings the mappings for the gamepad
	 */
	public Gamepad(Controller pad, GamepadMapping settings) {
		this.config = settings;
		this.pad = pad;

		for (Component comp : pad.getComponents()) {
			initValue(comp);
		}
	}

	/*
	 * Initializes the value of the given component to its current state
	 */
	private void initValue(Component comp) {
		handleComponent(comp, comp.getPollData());
	}

	/**
	 * Polls the gamepad for component values.
	 * @return true if the gamepad was polled successfully, false otherwise
	 */
	public boolean poll() {
		return pad.poll();
	}

	/*
	 * Sends a controller packet to the specified connection containing the current gamepad values
	 */
	private void sendControllerPacket(NvConnection conn) {
		if (conn != null) {
			conn.sendControllerInput(inputMap, leftTrigger, rightTrigger, 
					leftStickX, leftStickY, rightStickX, rightStickY);
		}
	}

	/**
	 * Gets the event queue for this gamepad
	 * @return this gamepad's event queue
	 */
	public EventQueue getEvents() {
		return pad.getEventQueue();
	}

	/**
	 * Handles the events in this gamepad's event queue and sends them
	 * to the specified connection
	 * @param conn the connection to the host that will receive the events
	 */
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
	 * Prints out the specified event information for the given gamepad
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

	/*
	 * Handles a given event
	 */
	private void handleEvent(Event event) {
		Component comp = event.getComponent();
		float value = event.getValue();

		handleComponent(comp, value);
	}

	/*
	 * Handles the component that an event occurred on
	 */
	private void handleComponent(Component comp, float value) {
		Mapping mapping = config.get(comp);
		if (mapping != null) {
			if (mapping.contComp.isAnalog()) {
				handleAnalog(mapping.contComp, sanitizeValue(mapping, value));
			} else {
				handleButtons(mapping.contComp, sanitizeValue(mapping, value));
			}
		}
	}

	/*
	 * Fixes the value as specified in the mapping, that is inverts if needed, etc.
	 */
	private float sanitizeValue(Mapping mapping, float value) {
		float sanitized = value;
		if (mapping.invert) {
			sanitized = -sanitized;
		}
		if (mapping.trigger) {
			sanitized = (sanitized + 1)/2;
		}
		return sanitized;
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

	/*
	 * Handles analog component input
	 */
	private void handleAnalog(GamepadComponent contComp, float value) {
		switch (contComp) {
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
			System.out.println("A mapping error has occured. Ignoring: " + contComp.name());
			break;
		}
	}
	
	/*
	 * Handles button input
	 */
	private void handleButtons(GamepadComponent contComp, float value) {
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
