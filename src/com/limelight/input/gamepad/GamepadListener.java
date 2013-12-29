package com.limelight.input.gamepad;

import java.lang.reflect.Constructor;
import java.util.LinkedList;

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
	
	/**
	 * starts a thread to listen to controllers
	 * @return true if it started a thread, false if the thread is already running.
	 */
	public static boolean startUp() {
		if (listenerThread == null || !listenerThread.isAlive()) {
			System.out.println("Controller Listener thread starting up");
			listenerThread = new Thread() {
				@Override
				public void run() {

					/*
					 * This is really janky, but it is currently the only way to rescan for controllers.
					 * The DefaultControllerEnvironment class caches the results of scanning and if a controller is
					 * unplugged or plugged in, it will not detect it. Since DefaultControllerEnvironment is package-protected
					 * we have to use reflections in order to manually instantiate a new instance to ensure there is no caching.
					 * Supposedly Aaron is going to fix JInput and we will have the ability to rescan soon! 
					 */
					try {
						//#allthejank
						Constructor<? extends ControllerEnvironment> construct = null;

						Class<? extends ControllerEnvironment> defEnv = ControllerEnvironment.getDefaultEnvironment().getClass();
						construct = defEnv.getDeclaredConstructor();
						construct.setAccessible(true);
						
						ControllerEnvironment defaultEnv = null;
						
						while(!isInterrupted()) {
							
							defaultEnv = (ControllerEnvironment)construct.newInstance();
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
