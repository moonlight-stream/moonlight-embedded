package com.limelight;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.UIManager;

import com.limelight.binding.PlatformBinding;
import com.limelight.gui.MainFrame;
import com.limelight.gui.StreamFrame;
import com.limelight.input.ControllerListener;
import com.limelight.nvstream.NvConnection;
import com.limelight.nvstream.NvConnectionListener;
import com.limelight.nvstream.StreamConfiguration;
import com.limelight.nvstream.av.video.VideoDecoderRenderer;
import com.limelight.settings.PreferencesManager;
import com.limelight.settings.PreferencesManager.Preferences;
import com.limelight.settings.PreferencesManager.Preferences.Resolution;

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

	private static void extractNativeLibrary(String libraryName, String targetDirectory) throws IOException {
		InputStream resource = new Object().getClass().getResourceAsStream("/binlib/"+libraryName);
		if (resource == null) {
			throw new FileNotFoundException("Unable to find native library in JAR: "+libraryName);
		}
		File destination = new File(targetDirectory+File.separatorChar+libraryName);
		
		// this will only delete it if it exists, and then create a new file
		destination.delete();
		destination.createNewFile();
		
		//this is the janky java 6 way to copy a file
		FileOutputStream fos = null;
		try {
			fos = new FileOutputStream(destination);
			int read;
			byte[] readBuffer = new byte[16384];
			while ((read = resource.read(readBuffer)) != -1) {
				fos.write(readBuffer, 0, read);
			}
		} finally {
			if (fos != null) {
				fos.close();
			}
		}
	}

	private static void prepareNativeLibraries() throws IOException {
		if (!System.getProperty("os.name").contains("Windows")) {
			// Nothing to do for platforms other than Windows
			return;
		}

		// We need to extract nv_avc_dec's runtime dependencies manually
		// because the current JRE extracts them with different file names
		// so they don't load properly.
		String nativeLibDir = ".";
		extractNativeLibrary("avfilter-3.dll", nativeLibDir);
		extractNativeLibrary("avformat-55.dll", nativeLibDir);
		extractNativeLibrary("avutil-52.dll", nativeLibDir);
		extractNativeLibrary("postproc-52.dll", nativeLibDir);
		extractNativeLibrary("pthreadVC2.dll", nativeLibDir);
		extractNativeLibrary("swresample-0.dll", nativeLibDir);
		extractNativeLibrary("swscale-2.dll", nativeLibDir);
		extractNativeLibrary("avcodec-55.dll", nativeLibDir);
	}

	private void startUp() {
		streamFrame = new StreamFrame();
		
		Preferences prefs = PreferencesManager.getPreferences();
		StreamConfiguration streamConfig = createConfiguration(prefs.getResolution());
		
		conn = new NvConnection(host, this, streamConfig);
		streamFrame.build(conn, streamConfig, prefs.getFullscreen());
		conn.start(PlatformBinding.getDeviceName(), streamFrame,
				VideoDecoderRenderer.FLAG_PREFER_QUALITY,
				PlatformBinding.getAudioRenderer(),
				PlatformBinding.getVideoDecoderRenderer());

		ControllerListener.startSendingInput(conn);

	}

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
	
	private static void startControllerListener() {
		ControllerListener.startUp();
	}

	private static void createFrame() {
		MainFrame main = new MainFrame();
		main.build();
		limeFrame = main.getLimeFrame();
		startControllerListener();
	}

	public static void createInstance(String host) {
		Limelight limelight = new Limelight(host);
		limelight.startUp();
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

		try {
			prepareNativeLibraries();
		} catch (IOException e) {
			// This is expected to fail when not in a JAR
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
		ControllerListener.startSendingInput(conn);
	}

	@Override
	public void connectionTerminated(Exception e) {
		e.printStackTrace();
		if (!connectionFailed) {
			connectionFailed = true;

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

	@Override
	public void displayMessage(String message) {
		JOptionPane.showMessageDialog(limeFrame, message, "Limelight", JOptionPane.INFORMATION_MESSAGE);
	}	

	public void displayError(String title, String message) {
		JOptionPane.showMessageDialog(limeFrame, message, title, JOptionPane.ERROR_MESSAGE);
	}
}

