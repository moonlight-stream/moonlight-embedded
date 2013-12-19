package com.limelight.input;

import javax.swing.JLabel;
import javax.swing.JTextField;

public enum ControllerComponent {
	BTN_A("Button 1 (A)"), BTN_X("Button 2 (X)"), BTN_Y("Button 3 (Y)"), BTN_B("Button 4 (B)"), 
	DPAD_UP("D-pad Up"), DPAD_DOWN("D-pad Down"), DPAD_LEFT("D-pad Left"), DPAD_RIGHT("D-pad Right"),
	LS_X("Left Stick X"), LS_Y("Left Stick X"), RS_X("Right Stick X"), RS_Y("Left Stick Y"),
	LS_THUMB("Left Stick Button"), RS_THUMB("Right Stick Button"),
	LT("Left Trigger"), RT("Right Trigger"), LB("Left Bumper"), RB("Right Bumper"), 
	BTN_START("Start Button"), BTN_BACK("Back Button"), BTN_SPECIAL("Special Button");
	
	private JLabel label;
	private JTextField textBox;
	
	private ControllerComponent(String name) {
		this.label = new JLabel(name);
		this.textBox = new JTextField();
		this.textBox.setEditable(false);
		this.textBox.setName(this.name());
	}
	
	public JLabel getLabel() {
		return label;
	}
	
	public JTextField getTextField() {
		return textBox;
	}
}
