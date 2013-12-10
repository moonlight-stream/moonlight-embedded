package com.limelight;

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

	public Limelight(String host) {
		this.host = host;
	}

	private void startUp(boolean fullscreen) {
		streamFrame = new StreamFrame();
		conn = new NvConnection(host, this);
		conn.start(PlatformBinding.getDeviceName(), streamFrame,
				VideoDecoderRenderer.FLAG_PREFER_QUALITY,
				PlatformBinding.getAudioRenderer(),
				PlatformBinding.getVideoDecoderRenderer());
		streamFrame.build(conn, fullscreen);

		startControllerListener();
	}

	private void startControllerListener() {
		new Thread(new Runnable() {
			@Override
			public void run() {
				Controller[] ca = ControllerEnvironment.getDefaultEnvironment().getControllers();
				System.out.println("found " + ca.length + " controllers");
				for(int i =0; i < ca.length; i++){
					if (ca[i].getType() == Controller.Type.GAMEPAD) {
						System.out.println("found a gamepad: " + ca[i].getName());
						GamepadHandler.addGamepad(ca[i], conn);
					}
				}
			}
		}).start();
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

			// set the look and feel
			try {
				UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
			} catch (Exception e) {
				System.out.println("OH Shit...");
				e.printStackTrace();
				System.exit(1);
			};
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
	}

	@Override
	public void connectionTerminated(Exception e) {
		e.printStackTrace();
		if (!connectionFailed) {
			connectionFailed = true;
			streamFrame.dispose();
			displayError("Connection Terminated", "The connection failed unexpectedly");
			conn.stop();
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

