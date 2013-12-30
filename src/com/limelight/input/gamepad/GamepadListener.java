package com.limelight.input.gamepad;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.logging.Level;
import java.util.logging.Logger;

import com.limelight.nvstream.NvConnection;

import net.java.games.input.Controller;
import net.java.games.input.ControllerEnvironment;

/**
 * Listens to <code>Controller</code>s connected to this computer and gives any gamepad to the gamepad handler
 * @author Diego Waxemberg
 */
public class GamepadListener {
	private static Thread listenerThread;
	private static NvConnection conn;
	private static Field controllers;
	private static Method scan;
	
	/**
	 * starts a thread to listen to controllers
	 * @return true if it started a thread, false if the thread is already running.
	 */
	public static boolean startUp() {
		// Suppress spam from jinput log warnings in DefaultControllerEnvironment
		Logger.getLogger(ControllerEnvironment.getDefaultEnvironment().getClass().getName()).setLevel(Level.SEVERE);
		
		if (listenerThread == null || !listenerThread.isAlive()) {
			System.out.println("Controller Listener thread starting up");
			listenerThread = new Thread() {
				@Override
				public void run() {

					/*
					 * This is really janky, but it is currently the only way to rescan for controllers.
					 * The DefaultControllerEnvironment class caches the results of scanning and if a controller is
					 * unplugged or plugged in, it will not detect it. Since DefaultControllerEnvironment is package-protected
					 * we have to use reflections in order to manually get access to the private field caching the controllers 
					 * and the method to scan for controllers
					 */
					try {
						//#allthejank
						
						ControllerEnvironment defaultEnv = ControllerEnvironment.getDefaultEnvironment();
						//must be called so the controllers array list will not be null
						defaultEnv.getControllers();
						
						controllers = defaultEnv.getClass().getDeclaredField("controllers");
						controllers.setAccessible(true);
						
						scan = defaultEnv.getClass().getDeclaredMethod("scanControllers", (Class[])null);
						scan.setAccessible(true);
						
						while(!isInterrupted()) {
							
							rescanControllers(defaultEnv);
							
							Controller[] ca = defaultEnv.getControllers();
							LinkedList<Controller> gamepads = new LinkedList<Controller>();

							/*
							 * iterates through the controllers and adds gamepads and ps3 controller to the list
							 * NOTE: JInput does not consider a PS3 controller to be a gamepad (it thinks it's a "STICK") so we must use the
							 * name of it.
							 */
							for(int i = 0; i < ca.length; i++){
								if (ca[i].getType() == Controller.Type.GAMEPAD) {
									gamepads.add(ca[i]);
								}	else if (ca[i].getName().contains("PLAYSTATION")) {
									gamepads.add(ca[i]);
								}
							}

							GamepadHandler.addGamepads(gamepads);
							GamepadHandler.setConnection(conn);
							
							try {
								Thread.sleep(1000);
							} catch (InterruptedException e) {
								return;
							}
						}
					} catch (Exception e) {
						e.printStackTrace();
					}
				}
			};
			listenerThread.start();
			return true;
		}
		return false;
	}

	/*
	 * Janky method that clears the controller environment's list of controllers and tells it to scan for new ones.
	 * #allthejank
	 */
	@SuppressWarnings("rawtypes")
	private static void rescanControllers(ControllerEnvironment defaultEnv) {
		try {
			((ArrayList)controllers.get(defaultEnv)).clear();
			scan.invoke(defaultEnv, (Object[])null);
		} catch (IllegalArgumentException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (SecurityException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IllegalAccessException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (InvocationTargetException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	/**
	 * Stops listening for controllers, ie. stops the thread if it is running
	 */
	public static void stopListening() {
		if (listenerThread != null && listenerThread.isAlive()) {
			System.out.println("Stopping Controller Listener thread");
			listenerThread.interrupt();
		}
		if (GamepadHandler.isRunning()) {
			GamepadHandler.stopHandler();
		}
	}
	
	/**
	 * Tells the handler to start sending gamepad events to the specified connection
	 * @param connection the connection to the host that will receive gamepad events
	 */
	public static void startSendingInput(NvConnection connection) {
		System.out.println("Starting to send controller input");
		conn = connection;
		if (!GamepadHandler.isRunning()) {
			GamepadHandler.startUp();
		}
	}
	
	/**
	 * Tells the handler to stop sending events to the host
	 */
	public static void stopSendingInput() {
		System.out.println("Stopping sending controller input");
		conn = null;
	}

}
