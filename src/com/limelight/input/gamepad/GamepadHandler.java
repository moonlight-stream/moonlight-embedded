package com.limelight.input.gamepad;


import java.util.Collections;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import com.limelight.nvstream.NvConnection;
import com.limelight.settings.GamepadSettingsManager;

import net.java.games.input.Controller;

/**
 * A handler for all the gamepads attached to this system
 * @author Diego Waxemberg
 */
public class GamepadHandler {
	private static LinkedList<Gamepad> gamepads = new LinkedList<Gamepad>();
	private static Lock gamepadLock = new ReentrantLock();
	private static NvConnection conn;
	private static Thread handler;
	private static boolean run = true;
	
	/**
	 * Adds the list of controllers as gamepads to be handled
	 * @param pads the controllers to be handled as gamepads
	 */
	public static void addGamepads(List<Controller> pads) {
		LinkedList<Gamepad> newPadList = new LinkedList<Gamepad>();
		
		GamepadMapping settings = GamepadSettingsManager.getSettings();
		for (Controller pad : pads) {

			newPadList.add(new Gamepad(pad, settings));
		}
		
		gamepadLock.lock();
		gamepads = newPadList;
		gamepadLock.unlock();
	}

	/**
	 * Sets the connection to which the handled gamepads should send events
	 * @param connection the connection to the host
	 */
	public static void setConnection(NvConnection connection) {
		conn = connection;
	}

	/**
	 * Gets a list of the currently handled gamepads
	 * @return an unmodifiable list of gamepads this handler handles
	 */
	public static List<Gamepad> getGamepads() {
		return Collections.unmodifiableList(gamepads);
	}

	/**
	 * Starts up a thread that handles the gamepads
	 */
	public static void startUp() {
		if (handler == null || !handler.isAlive()) {
			run = true;
			System.out.println("Gamepad Handler thread starting up");
			handler = new Thread(new Runnable() {
				@Override
				public void run() {
					while (run) {
						try {
							gamepadLock.lockInterruptibly();
						} catch (InterruptedException e1) {
							run = false;
						}
						for (Gamepad gamepad : gamepads) {
							if (!gamepad.poll()) {
								break;
							}
							gamepad.handleEvents(conn);
						}
						gamepadLock.unlock();
						try {
							Thread.sleep(20);
						} catch (InterruptedException e) {
							run = false;
						}
					}
				}
			});
			handler.start();
		}
	}

	/**
	 * Stops handling the gamepads
	 */
	public static void stopHandler() {
		if (handler != null && handler.isAlive()) {
			System.out.println("Stopping Gamepad Handler thread");
			handler.interrupt();
			conn = null;
		}
	}
	
	/**
	 * Checks if the handler is handling the gamepads.
	 * @return whether this handler is running
	 */
	public static boolean isRunning() {
		return (handler != null && handler.isAlive());
	}
	
}
