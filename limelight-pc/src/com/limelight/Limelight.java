package com.limelight;

import javax.swing.JOptionPane;
import javax.swing.UIManager;
import com.limelight.binding.PlatformBinding;
import com.limelight.gui.MainFrame;
import com.limelight.gui.StreamFrame;
import com.limelight.nvstream.NvConnection;
import com.limelight.nvstream.NvConnectionListener;
import com.limelight.nvstream.av.video.VideoDecoderRenderer;

public class Limelight implements NvConnectionListener {
	public static final double VERSION = 1.0;

	private String host;
	private StreamFrame streamFrame;
	private NvConnection conn;
	
	public Limelight(String host) {
		this.host = host;
	}

	private void startUp() {
		streamFrame = new StreamFrame();
		conn = new NvConnection(host, this);
		conn.start(PlatformBinding.getDeviceName(), streamFrame,
				VideoDecoderRenderer.FLAG_PREFER_QUALITY,
				PlatformBinding.getAudioRenderer(),
				PlatformBinding.getVideoDecoderRenderer());
		streamFrame.build(conn);
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

			// set the look and feel
			try {
				UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
			} catch (Exception e) {
				System.out.println("OH Shit...");
				e.printStackTrace();
				System.exit(1);
			};
		}

		MainFrame limeFrame = new MainFrame();
		limeFrame.build();
	}

	@Override
	public void stageStarting(Stage stage) {
		System.out.println("Starting "+stage.getName());
	}

	@Override
	public void stageComplete(Stage stage) {
	}

	@Override
	public void stageFailed(Stage stage) {
		JOptionPane.showMessageDialog(streamFrame, "Starting "+stage.getName()+" failed", "Connection Error", JOptionPane.ERROR_MESSAGE);
		conn.stop();
	}

	@Override
	public void connectionStarted() {
	}

	@Override
	public void connectionTerminated(Exception e) {
		e.printStackTrace();
		JOptionPane.showMessageDialog(streamFrame, "The connection failed unexpectedly", "Connection Terminated", JOptionPane.ERROR_MESSAGE);
		conn.stop();
	}

	@Override
	public void displayMessage(String message) {
		JOptionPane.showMessageDialog(streamFrame, message, "Limelight", JOptionPane.INFORMATION_MESSAGE);
	}
}

