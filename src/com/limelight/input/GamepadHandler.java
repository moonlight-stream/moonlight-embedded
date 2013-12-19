package com.limelight.input;


import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

import javax.swing.event.ListSelectionEvent;

import com.limelight.input.Gamepad.ControllerType;
import com.limelight.nvstream.NvConnection;

import net.java.games.input.Controller;

public class GamepadHandler {
	private static LinkedList<Gamepad> gamepads = new LinkedList<Gamepad>();
	private static GamepadHandler singleton;

	public static void addGamepads(List<Controller> pads, NvConnection conn) {
		if (singleton == null) {
			singleton = new GamepadHandler();
			singleton.startUp();
		}

		gamepads.clear();

		for (Controller pad : pads) {
		
			gamepads.add(Gamepad.createInstance(conn, pad, getType(pad)));
		}
	}

	private static ControllerType getType(Controller pad) {
		if (pad.getType() == Controller.Type.GAMEPAD) {
			return ControllerType.XBOX;
		}
		if (pad.getName().contains("PLAYSTATION")) {
			return ControllerType.PS3;
		}
		return null;
	}
	
	public static List<Gamepad> getGamepads() {
		return Collections.unmodifiableList(gamepads);
	}
	
	private void startUp() {
		new Thread(new Runnable() {
			@Override
			public void run() {
				while (true) {
					for (Gamepad gamepad : gamepads) {
						if (!gamepad.poll()) {
							break;
						}
						gamepad.handleEvents();
					}
					try {
						Thread.sleep(20);
					} catch (InterruptedException e) {}
				}
			}
		}).start();
	}
	
}
