package com.limelight.input;


import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

import com.limelight.nvstream.NvConnection;
import com.limelight.nvstream.input.ControllerPacket;

import net.java.games.input.Component;
import net.java.games.input.Controller;
import net.java.games.input.Event;
import net.java.games.input.EventQueue;

public class GamepadHandler {
	private NvConnection conn;

	private static LinkedList<Controller> gamepads = new LinkedList<Controller>();
	private static GamepadHandler singleton;

	short inputMap = 0x0000;
	private byte leftTrigger = 0x00;
	private byte rightTrigger = 0x00;
	private short rightStickX = 0x0000;
	private short rightStickY = 0x0000;
	private short leftStickX = 0x0000;
	private short leftStickY = 0x0000;

	private GamepadHandler(NvConnection conn) {
		this.conn = conn;
	}

	public static void addGamepads(List<Controller> pads, NvConnection conn) {
		if (singleton == null) {
			singleton = new GamepadHandler(conn);
			singleton.startUp();
		}

		gamepads.clear();

		for (Controller pad : pads) {
			gamepads.add(pad);
		}
	}

	private void startUp() {
		new Thread(new Runnable() {
			@Override
			public void run() {
				while (true) {
					for (Controller gamepad : gamepads) {
						if (!gamepad.poll()) {
							gamepads.remove(GamepadHandler.this);
							break;
						}
						EventQueue queue = gamepad.getEventQueue();
						Event event = new Event();

						while(queue.getNextEvent(event)) {

							//printInfo(gamepda, event);

							handleEvent(event);

							sendControllerPacket();
						}
					}
					try {
						Thread.sleep(20);
					} catch (InterruptedException e) {}
				}
			}
		}).start();
	}
	
	private void printInfo(Controller gamepad, Event event) {
		StringBuffer buffer = new StringBuffer(gamepad.getName());
		buffer.append(" at ");
		buffer.append(event.getNanos()).append(", ");
		Component comp = event.getComponent();
		buffer.append(comp.getName()).append(" changed to ");
		float value = event.getValue(); 
		if(comp.isAnalog()) {
			buffer.append(value);
		} else {
			if(value==1.0f) {
				buffer.append("On");
			} else {
				buffer.append("Off");
			}
		}
		System.out.println(buffer.toString());
		System.out.println(comp.getIdentifier().toString());
	}


	private void handleEvent(Event event) {
		Component comp = event.getComponent();
		float value = event.getValue();

		if (comp.isAnalog()) {
			handleAnalog(comp, value);
		} else {
			handleButtons(comp, value);
		}
	}

	private void toggle(short button, boolean press) {
		if (press) {
			inputMap |= button;
		} else {
			inputMap &= ~button;
		}
	}

	private void handleButtons(Component comp, float value) {
		Component.Identifier id = comp.getIdentifier();
		boolean press = value > 0.5F;

		if (id == Component.Identifier.Button._13) {
			toggle(ControllerPacket.LEFT_FLAG, press);
		} else if (id == Component.Identifier.Button._14) {
			toggle(ControllerPacket.RIGHT_FLAG, press);
		} else if (id == Component.Identifier.Button._11) {
			toggle(ControllerPacket.UP_FLAG, press);
		} else if (id == Component.Identifier.Button._12) {
			toggle(ControllerPacket.DOWN_FLAG, press);
		} else if (id == Component.Identifier.Button._0) {
			toggle(ControllerPacket.A_FLAG, press);
		} else if (id == Component.Identifier.Button._2) {
			toggle(ControllerPacket.X_FLAG, press);
		} else if (id == Component.Identifier.Button._3) {
			toggle(ControllerPacket.Y_FLAG, press);
		} else if (id == Component.Identifier.Button._1) {
			toggle(ControllerPacket.B_FLAG, press);
		} else if (id == Component.Identifier.Button._9) {
			toggle(ControllerPacket.BACK_FLAG, press);
		} else if (id == Component.Identifier.Button._8) {
			toggle(ControllerPacket.PLAY_FLAG, press);
		} else if (id == Component.Identifier.Button._7) {
			toggle(ControllerPacket.RS_CLK_FLAG, press);
		} else if (id == Component.Identifier.Button._6) {
			toggle(ControllerPacket.LS_CLK_FLAG, press);
		} else if (id == Component.Identifier.Button._4) {
			toggle(ControllerPacket.LB_FLAG, press);
		} else if (id == Component.Identifier.Button._5) {
			toggle(ControllerPacket.RB_FLAG, press);
		} else if (id == Component.Identifier.Button._10) {
			toggle(ControllerPacket.SPECIAL_BUTTON_FLAG, press);
		}
	}

	private void handleAnalog(Component comp, float value) {
		Component.Identifier id = comp.getIdentifier();

		if (id == Component.Identifier.Axis.RX) {
			rightStickX = (short)Math.round(value * 0x7FFF);
		} else if (id == Component.Identifier.Axis.RY) {
			rightStickY = (short)Math.round(-value * 0x7FFF);
		} else if (id == Component.Identifier.Axis.X) {
			leftStickX = (short)Math.round(value * 0x7FFF);
		} else if (id == Component.Identifier.Axis.Y) {
			leftStickY = (short)Math.round(-value * 0x7FFF);
		} else if (id == Component.Identifier.Axis.Z) {
			leftTrigger = (byte)Math.round((value + 1 / 2) * 0xFF);
		} else if (id == Component.Identifier.Axis.RZ) {
			rightTrigger = (byte)Math.round((value + 2) / 2 * 0xFF);
		}

	}

	private void sendControllerPacket() {
		conn.sendControllerInput(inputMap, leftTrigger, rightTrigger, 
				leftStickX, leftStickY, rightStickX, rightStickY);
	}
}
