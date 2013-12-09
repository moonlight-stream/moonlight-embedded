package com.limelight.input;


import java.util.ArrayList;

import com.limelight.nvstream.NvConnection;
import com.limelight.nvstream.input.ControllerPacket;

import net.java.games.input.Component;
import net.java.games.input.Controller;
import net.java.games.input.Event;
import net.java.games.input.EventQueue;

public class GamepadHandler {
	private NvConnection conn;

	private Controller gamepad;
	private static ArrayList<GamepadHandler> gamepads;
	private boolean run = true;

	short inputMap = 0x0000;
	private byte leftTrigger = 0x00;
	private byte rightTrigger = 0x00;
	private short rightStickX = 0x0000;
	private short rightStickY = 0x0000;
	private short leftStickX = 0x0000;
	private short leftStickY = 0x0000;

	private GamepadHandler(Controller gamepad, NvConnection conn) {
		this.gamepad = gamepad;
		this.conn = conn;
	}

	public static void addGamepad(Controller pad, NvConnection conn) {
		if (gamepads == null) {
			gamepads = new ArrayList<GamepadHandler>();
		}
		if (!gamepads.contains(pad)) {
			System.out.println("adding gamepad");
			GamepadHandler gh = new GamepadHandler(pad, conn);
			gh.startUp();
			gamepads.add(gh);
		}
	}
	
	
	private void startUp() {
		new Thread(new Runnable() {
			@Override
			public void run() {
				while (run) {
					if (!gamepad.poll()) {
						run = false;
						gamepads.remove(GamepadHandler.this);
					}
					EventQueue queue = gamepad.getEventQueue();
					Event event = new Event();

					while(queue.getNextEvent(event)) {
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

						if (comp.getName().equals("rx")) {
							rightStickX = (short)Math.round(event.getValue() * 0x7FFF);
						}
						if (comp.getName().equals("ry")) {
							rightStickY = (short)Math.round(-event.getValue() * 0x7FFF);
						}
						if (comp.getName().equals("x")) {
							leftStickX = (short)Math.round(event.getValue() * 0x7FFF);
						}
						if (comp.getName().equals("y")) {
							leftStickY = (short)Math.round(-event.getValue() * 0x7FFF);
						}
						if (comp.getName().equals("z")) {
							leftTrigger = (byte)Math.round((event.getValue() + 1) / 2 * 0xFF);
						}
						if (comp.getName().equals("rz")) {
							rightTrigger = (byte)Math.round((event.getValue() + 1) / 2 * 0xFF);
						}
						if (comp.getName().equals("13")) {
							if (event.getValue() == 1.0F) {
								inputMap |= ControllerPacket.LEFT_FLAG;
							} else {
								inputMap &= ~ControllerPacket.LEFT_FLAG;
							}
						}
						if (comp.getName().equals("14")) {
							if (event.getValue() == 1.0F) {
								inputMap |= ControllerPacket.RIGHT_FLAG;
							} else {
								inputMap &= ~ControllerPacket.RIGHT_FLAG;
							}
						}
						if (comp.getName().equals("11")) {
							if (event.getValue() == 1.0F) {
								inputMap |= ControllerPacket.UP_FLAG;
							} else {
								inputMap &= ~ControllerPacket.UP_FLAG;
							}
						}
						if (comp.getName().equals("12")) {
							if (event.getValue() == 1.0F) {
								inputMap |= ControllerPacket.DOWN_FLAG;
							} else {
								inputMap &= ~ControllerPacket.DOWN_FLAG;
							}
						}
						if (comp.getName().equals("0")) {
							if (event.getValue() == 1.0F) {
								inputMap |= ControllerPacket.A_FLAG;
							} else {
								inputMap &= ~ControllerPacket.A_FLAG;
							}
						}
						if (comp.getName().equals("2")) {
							if (event.getValue() == 1.0F) {
								inputMap |= ControllerPacket.X_FLAG;
							} else {
								inputMap &= ~ControllerPacket.X_FLAG;
							}
						}
						if (comp.getName().equals("3")) {
							if (event.getValue() == 1.0F) {
								inputMap |= ControllerPacket.Y_FLAG;
							} else {
								inputMap &= ~ControllerPacket.Y_FLAG;
							}
						}
						if (comp.getName().equals("1")) {
							if (event.getValue() == 1.0F) {
								inputMap |= ControllerPacket.B_FLAG;
							} else {
								inputMap &= ~ControllerPacket.B_FLAG;
							}
						}
						if (comp.getName().equals("4")) {
							if (event.getValue() == 1.0F) {
								inputMap |= ControllerPacket.LB_FLAG;
							} else {
								inputMap &= ~ControllerPacket.LB_FLAG;
							}
						}
						if (comp.getName().equals("5")) {
							if (event.getValue() == 1.0F) {
								inputMap |= ControllerPacket.RB_FLAG;
							} else {
								inputMap &= ~ControllerPacket.RB_FLAG;
							}
						}
						if (comp.getName().equals("9")) {
							if (event.getValue() == 1.0F) {
								inputMap |= ControllerPacket.BACK_FLAG;
							} else {
								inputMap &= ~ControllerPacket.BACK_FLAG;
							}
						}
						if (comp.getName().equals("8")) {
							if (event.getValue() == 1.0F) {
								inputMap |= ControllerPacket.PLAY_FLAG;
							} else {
								inputMap &= ~ControllerPacket.PLAY_FLAG;
							}
						}
						if (comp.getName().equals("10")) {
							if (event.getValue() == 1.0F) {
								inputMap |= ControllerPacket.SPECIAL_BUTTON_FLAG;
							} else {
								inputMap &= ~ControllerPacket.SPECIAL_BUTTON_FLAG;
							}
						}
						if (comp.getName().equals("6")) {
							if (event.getValue() == 1.0F) {
								inputMap |= ControllerPacket.LS_CLK_FLAG;
							} else {
								inputMap &= ~ControllerPacket.LS_CLK_FLAG;
							}
						}
						if (comp.getName().equals("7")) {
							if (event.getValue() == 1.0F) {
								inputMap |= ControllerPacket.RS_CLK_FLAG;
							} else {
								inputMap &= ~ControllerPacket.RS_CLK_FLAG;
							}
						}
						
						sendControllerPacket();
					}

					try {
						Thread.sleep(20);
					} catch (InterruptedException e) {}
				}
			}
		}).start();
	}

	private void sendControllerPacket() {
		conn.sendControllerInput(inputMap, leftTrigger, rightTrigger, 
				leftStickX, leftStickY, rightStickX, rightStickY);
	}
}
