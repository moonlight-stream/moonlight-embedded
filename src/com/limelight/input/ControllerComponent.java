package com.limelight.input;

import java.io.Serializable;

import javax.swing.JButton;
import javax.swing.JLabel;

public enum ControllerComponent implements Serializable {
	BTN_A("Button 1 (A)", false), BTN_X("Button 2 (X)", false), BTN_Y("Button 3 (Y)", false), BTN_B("Button 4 (B)", false), 
	DPAD_UP("D-pad Up", false), DPAD_DOWN("D-pad Down", false), DPAD_LEFT("D-pad Left", false), DPAD_RIGHT("D-pad Right", false),
	LS_X("Left Stick X", true), LS_Y("Left Stick Y", true), RS_X("Right Stick X", true), RS_Y("Right Stick Y", true),
	LS_THUMB("Left Stick Button", false), RS_THUMB("Right Stick Button", false),
	LT("Left Trigger", true), RT("Right Trigger", true), LB("Left Bumper", false), RB("Right Bumper", false), 
	BTN_START("Start Button", false), BTN_BACK("Back Button", false), BTN_SPECIAL("Special Button", false);
	
	private JLabel label;
	private JButton mapButton;
	private boolean analog;
	
	private ControllerComponent(String name, boolean analog) {
		this.label = new JLabel(name);
		this.mapButton = new JButton();
		this.mapButton.setName(this.name());
		this.analog = analog;
	}
	
	public JLabel getLabel() {
		return label;
	}
	
	public JButton getMapButton() {
		return mapButton;
	}
	
	public boolean isAnalog() {
		return analog;
	}
}
