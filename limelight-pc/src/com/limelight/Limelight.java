package com.limelight;

import javax.swing.JOptionPane;

import com.limelight.gui.MainFrame;
import com.limelight.gui.StreamFrame;
import com.limelight.nvstream.NvConnection;
import com.limelight.nvstream.NvConnectionListener;

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
		streamFrame.build();
		conn = new NvConnection(host, streamFrame, this);
		conn.start();
	}

	public static void createInstance(String host) {
		Limelight limelight = new Limelight(host);
		limelight.startUp();
	}

	public static void main(String args[]) {
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
}

