package com.limelight;

import java.lang.reflect.Constructor;
import java.util.LinkedList;

import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.UIManager;

import net.java.games.input.Controller;
import net.java.games.input.ControllerEnvironment;

import com.limelight.binding.PlatformBinding;
import com.limelight.gui.MainFrame;
import com.limelight.gui.StreamFrame;
import com.limelight.input.GamepadHandler;
import com.limelight.nvstream.NvConnection;
import com.limelight.nvstream.NvConnectionListener;
import com.limelight.nvstream.av.video.VideoDecoderRenderer;

public class Limelight implements NvConnectionListener {
	public static final double VERSION = 1.0;

	private String host;
	private StreamFrame streamFrame;
	private NvConnection conn;
	private boolean connectionFailed;
	private static JFrame limeFrame;
	private Thread controllerListenerThread;

	public Limelight(String host) {
		this.host = host;
	}

	private void startUp(boolean fullscreen) {
		streamFrame = new StreamFrame();
		conn = new NvConnection(host, this);
		streamFrame.build(conn, fullscreen);
		conn.start(PlatformBinding.getDeviceName(), streamFrame,
				VideoDecoderRenderer.FLAG_PREFER_QUALITY,
				PlatformBinding.getAudioRenderer(),
				PlatformBinding.getVideoDecoderRenderer());
		
	}

	private void startControllerListener() {
		controllerListenerThread = new Thread() {
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
						
						GamepadHandler.addGamepads(gamepads, conn);
						
						
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
		controllerListenerThread.start();
	}

	private static void createFrame() {
		MainFrame main = new MainFrame();
		main.build();
		limeFrame = main.getLimeFrame();
	}

	public static void createInstance(String host, boolean fullscreen) {
		Limelight limelight = new Limelight(host);
		limelight.startUp(fullscreen);
	}

	public static void main(String args[]) {
		//fix the menu bar if we are running in osx
		if (System.getProperty("os.name").contains("Mac OS X")) {
			// take the menu bar off the jframe
			System.setProperty("apple.laf.useScreenMenuBar", "true");

			// set the name of the application menu item
			System.setProperty("com.apple.mrj.application.apple.menu.about.name", "Limelight");

		} else {
			try {
				UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
			} catch (Exception e) {
				System.out.println("Unable to set cross platform look and feel.");
				e.printStackTrace();
				System.exit(2);
			}
		}
		createFrame();
	}

	@Override
	public void stageStarting(Stage stage) {
		System.out.println("Starting "+stage.getName());
		streamFrame.showSpinner(stage);
	}

	@Override
	public void stageComplete(Stage stage) {
	}

	@Override
	public void stageFailed(Stage stage) {
		streamFrame.dispose();
		conn.stop();
		displayError("Connection Error", "Starting " + stage.getName() + " failed");
	}

	@Override
	public void connectionStarted() {
		streamFrame.hideSpinner();
		startControllerListener();
	}

	@Override
	public void connectionTerminated(Exception e) {
		e.printStackTrace();
		if (!connectionFailed) {
			connectionFailed = true;
			
			// Kill the connection to the target
			conn.stop();
			
			// Kill the controller rescanning thread
			if (controllerListenerThread != null) {
				controllerListenerThread.interrupt();		
				try {
					controllerListenerThread.join();
				} catch (InterruptedException e1) {}
			}

			// Spin off a new thread to update the UI since
			// this thread has been interrupted and will terminate
			// shortly
			new Thread(new Runnable() {
				@Override
				public void run() {
					streamFrame.dispose();
					displayError("Connection Terminated", "The connection failed unexpectedly");
				}
			}).start();
		}
	}

	@Override
	public void displayMessage(String message) {
		JOptionPane.showMessageDialog(limeFrame, message, "Limelight", JOptionPane.INFORMATION_MESSAGE);
	}	

	public void displayError(String title, String message) {
		JOptionPane.showMessageDialog(limeFrame, message, title, JOptionPane.ERROR_MESSAGE);
	}
}

