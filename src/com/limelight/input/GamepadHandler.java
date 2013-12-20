package com.limelight.input;


import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

import com.limelight.nvstream.NvConnection;
import com.limelight.settings.GamepadSettingsManager;

import net.java.games.input.Controller;

public class GamepadHandler {
	private static LinkedList<Gamepad> gamepads = new LinkedList<Gamepad>();
	private static NvConnection conn;
	private static Thread handler;

	public static void addGamepads(List<Controller> pads) {

		gamepads.clear();

		
		GamepadSettings settings = GamepadSettingsManager.getSettings();
		for (Controller pad : pads) {

			gamepads.add(new Gamepad(pad, settings));
		}
	}

	public static void setConnection(NvConnection connection) {
		conn = connection;
	}

	public static List<Gamepad> getGamepads() {
		return Collections.unmodifiableList(gamepads);
	}

	public static void startUp() {
		if (handler == null || !handler.isAlive()) {
			System.out.println("Gamepad Handler thread starting up");
			handler = new Thread(new Runnable() {
				@Override
				public void run() {
					while (true) {
						for (Gamepad gamepad : gamepads) {
							if (!gamepad.poll()) {
								break;
							}
							
							gamepad.handleEvents(conn);
						}
						try {
							Thread.sleep(20);
						} catch (InterruptedException e) {}
					}
				}
			});
			handler.start();
		}
	}

	public static void stopHandler() {
		if (handler != null && handler.isAlive()) {
			System.out.println("Stopping Gamepad Handler thread");
			handler.interrupt();
			conn = null;
		}
	}
	
}
