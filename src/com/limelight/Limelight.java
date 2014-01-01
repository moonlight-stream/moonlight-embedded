package com.limelight;

import java.io.File;
import java.io.IOException;
import java.io.PrintStream;

import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.UIManager;

import com.limelight.binding.LibraryHelper;
import com.limelight.binding.PlatformBinding;
import com.limelight.gui.MainFrame;
import com.limelight.gui.StreamFrame;
import com.limelight.input.gamepad.Gamepad;
import com.limelight.input.gamepad.GamepadListener;
import com.limelight.input.gamepad.NativeGamepad;
import com.limelight.nvstream.NvConnection;
import com.limelight.nvstream.NvConnectionListener;
import com.limelight.nvstream.StreamConfiguration;
import com.limelight.nvstream.av.video.VideoDecoderRenderer;
import com.limelight.settings.PreferencesManager;
import com.limelight.settings.PreferencesManager.Preferences;
import com.limelight.settings.PreferencesManager.Preferences.Resolution;

/**
 * Main class for Limelight-pc contains methods for starting the application as well
 * as the stream to the host pc.
 * @author Diego Waxemberg<br>
 * Cameron Gutman
 */
public class Limelight implements NvConnectionListener {
	public static final double VERSION = 1.0;

	private String host;
	private StreamFrame streamFrame;
	private NvConnection conn;
	private boolean connectionTerminating;
	private static JFrame limeFrame;

	/**
	 * Constructs a new instance based on the given host
	 * @param host can be hostname or IP address.
	 */
	public Limelight(String host) {
		this.host = host;
	}

	/*
	 * Creates a connection to the host and starts up the stream.
	 */
	private void startUp() {
		streamFrame = new StreamFrame();
		
		Preferences prefs = PreferencesManager.getPreferences();
		StreamConfiguration streamConfig = createConfiguration(prefs.getResolution());
		
		conn = new NvConnection(host, this, streamConfig);
		streamFrame.build(this, conn, streamConfig, prefs.getFullscreen());
		conn.start(PlatformBinding.getDeviceName(), streamFrame,
				VideoDecoderRenderer.FLAG_PREFER_QUALITY,
				PlatformBinding.getAudioRenderer(),
				PlatformBinding.getVideoDecoderRenderer());
		
		GamepadListener.getInstance().addDeviceListener(new Gamepad(conn));
	}

	/*
	 * Creates a StreamConfiguration given a Resolution. 
	 * Used to specify what kind of stream will be used.
	 */
	private StreamConfiguration createConfiguration(Resolution res) {
		switch(res) {
		case RES_720_30:
			return new StreamConfiguration(1280, 720, 30);
		case RES_720_60:
			return new StreamConfiguration(1280, 720, 60);
		case RES_1080_30:
			return new StreamConfiguration(1920, 1080, 30);
		case RES_1080_60:
			return new StreamConfiguration(1920, 1080, 60);
		default:
			// this should never happen, if it does we want the NPE to occur so we know something is wrong
			return null;
		}
	}
	
	/*
	 * Creates the main frame for the application.
	 */
	private static void createFrame() {
		MainFrame main = new MainFrame();
		main.build();
		limeFrame = main.getLimeFrame();
	}

	/**
	 * Creates a new instance and starts the stream.
	 * @param host the host pc to connect to. Can be a hostname or IP address.
	 */
	public static void createInstance(String host) {
		Limelight limelight = new Limelight(host);
		limelight.startUp();
	}

	/**
	 * The entry point for the application. <br>
	 * Does some initializations and then creates the main frame.
	 * @param args unused.
	 */
	//TODO: We should allow command line args to specify things like debug mode (verbose logging) or even start a stream directly.
	public static void main(String args[]) {
		// Redirect logging to a file if we're running from a JAR
		if (LibraryHelper.isRunningFromJar()) {
			try {
				System.setErr(new PrintStream(new File("error.log")));
				System.setOut(new PrintStream(new File("output.log")));
			} catch (IOException e) {
			}
		}
		
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

		LibraryHelper.prepareNativeLibraries();

		createFrame();
		
		NativeGamepad.addListener(GamepadListener.getInstance());
		NativeGamepad.start();
	}
	
	public void stop() {
		connectionTerminating = true;
		conn.stop();
	}

	/**
	 * Callback to specify which stage is starting. Used to update UI.
	 * @param stage the Stage that is starting
	 */
	@Override
	public void stageStarting(Stage stage) {
		System.out.println("Starting "+stage.getName());
		streamFrame.showSpinner(stage);
	}

	/**
	 * Callback that a stage has finished loading.
	 * <br><b>NOTE: Currently unimplemented.</b>
	 * @param stage the Stage that has finished.
	 */
	@Override
	public void stageComplete(Stage stage) {
	}

	/**
	 * Callback that a stage has failed. Used to inform user that an error occurred.
	 * @param stage the Stage that was loading when the error occurred
	 */
	@Override
	public void stageFailed(Stage stage) {
		streamFrame.dispose();
		conn.stop();
		displayError("Connection Error", "Starting " + stage.getName() + " failed");
	}

	/**
	 * Callback that the connection has finished loading and is started.
	 */
	@Override
	public void connectionStarted() {
		streamFrame.hideSpinner();
	}

	/**
	 * Callback that the connection has been terminated for some reason.
	 * <br>This is were the stream shutdown procedure takes place.
	 * @param e the Exception that was thrown- probable cause of termination.
	 */
	@Override
	public void connectionTerminated(Exception e) {
		if (!(e instanceof InterruptedException)) {
			e.printStackTrace();
		}
		if (!connectionTerminating) {
			connectionTerminating = true;

			// Kill the connection to the target
			conn.stop();

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

	/**
	 * Displays a message to the user in the form of an info dialog.
	 * @param message the message to show the user
	 */
	@Override
	public void displayMessage(String message) {
		JOptionPane.showMessageDialog(limeFrame, message, "Limelight", JOptionPane.INFORMATION_MESSAGE);
	}	

	/**
	 * Displays an error to the user in the form of an error dialog
	 * @param title the title for the dialog frame
	 * @param message the message to show the user
	 */
	public void displayError(String title, String message) {
		JOptionPane.showMessageDialog(limeFrame, message, title, JOptionPane.ERROR_MESSAGE);
	}
}

