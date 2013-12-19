package com.limelight.input;

import java.lang.reflect.Constructor;
import java.util.LinkedList;

import com.limelight.nvstream.NvConnection;

import net.java.games.input.Controller;
import net.java.games.input.ControllerEnvironment;

public class ControllerListener {
	private static Thread listenerThread;
	private static NvConnection conn;
	
	/**
	 * starts a thread to listen to controllers
	 * @return true if it started a thread, false if the thread is already running.
	 */
	public static boolean startUp() {
		if (listenerThread == null || !listenerThread.isAlive()) {
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

						while(!isInterrupted()) {

							ControllerEnvironment defaultEnv = null;

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
							if (conn != null) {
								GamepadHandler.setConnection(conn);
							}
							
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

	public static void stopListening() {
		if (listenerThread != null && listenerThread.isAlive()) {
			listenerThread.interrupt();
		}
	}
	
	public static void startSendingInput(NvConnection connection) {
		conn = connection;
	}

}
