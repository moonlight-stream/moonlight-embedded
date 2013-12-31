package com.limelight.input;

public class Device {
	private int id;
	private int numButtons;
	private int numAxes;
	
	public Device(int deviceID, int numButtons, int numAxes) {
		this.id = deviceID;
		this.numButtons = numButtons;
		this.numAxes = numAxes;
	}
	
	public int getId() {
		return id;
	}
	
	public int getNumButtons() {
		return numButtons;
	}
	
	public int getNumAxes() {
		return numAxes;
	}
}
