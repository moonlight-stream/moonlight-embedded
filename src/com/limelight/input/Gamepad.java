package com.limelight.input;

import com.limelight.nvstream.NvConnection;

import net.java.games.input.Component;
import net.java.games.input.Controller;
import net.java.games.input.Event;
import net.java.games.input.EventQueue;

public abstract class Gamepad {
	protected Controller pad;
	private NvConnection conn;
	
	protected short inputMap = 0x0000;
	protected byte leftTrigger = 0x00;
	protected byte rightTrigger = 0x00;
	protected short rightStickX = 0x0000;
	protected short rightStickY = 0x0000;
	protected short leftStickX = 0x0000;
	protected short leftStickY = 0x0000;
	
	protected GamepadSettings configuration;
	
	public enum ControllerType { XBOX, PS3 };
	
	
	public static Gamepad createInstance(NvConnection conn, Controller pad, ControllerType type) {
		switch (type) {
		case XBOX:
			return new XBox360Controller(conn, pad);
		case PS3:
			return new PS3Controller(conn, pad);
		default:
			return new XBox360Controller(conn, pad);
		}
	}
	
	public Gamepad(NvConnection conn, Controller pad) {
		this.conn = conn;
		this.pad = pad;
		
		configuration = new GamepadSettings();
		
		for (Component comp : pad.getComponents()) {
			initValue(comp);
		}
		
	}
		
	public GamepadSettings getConfiguration() {
		return configuration;
	}
	
	private void initValue(Component comp) {
		handleComponent(comp, comp.getPollData());
	}
	
	public boolean poll() {
		return pad.poll();
	}
	
	public void sendControllerPacket() {
		conn.sendControllerInput(inputMap, leftTrigger, rightTrigger, 
				leftStickX, leftStickY, rightStickX, rightStickY);
	}
	
	public EventQueue getEvents() {
		return pad.getEventQueue();
	}
	
	public void handleEvents() {
		EventQueue queue = pad.getEventQueue();
		Event event = new Event();
		
		while(queue.getNextEvent(event)) {
			
			/* uncommented for debugging */
			//printInfo(pad, event);

			handleEvent(event);

			sendControllerPacket();
		}
		
	}
	
	/*
	 * used for debugging, normally unused.
	 */
	@SuppressWarnings("unused")
	private void printInfo(Controller gamepad, Event event) {
		Component comp = event.getComponent();
		
		StringBuilder builder = new StringBuilder(gamepad.getName());
		
		builder.append(" at ");
		builder.append(event.getNanos()).append(": ");
		builder.append(comp.getName()).append(" changed to ");
		
		
		float value = event.getValue(); 
		if(comp.isAnalog()) {
			builder.append(value);
		} else {
			if(value==1.0f) {
				builder.append("On");
			} else {
				builder.append("Off");
			}
		}
		
		System.out.println(builder.toString());
	}
	
	private void handleEvent(Event event) {
		Component comp = event.getComponent();
		float value = event.getValue();
		
		handleComponent(comp, value);
	}
	
	private void handleComponent(Component comp, float value) {
		if (comp.isAnalog()) {
			handleAnalog(comp, value);
		} else {
			handleButtons(comp, value);
		}
	}
	
	protected void toggle(short button, boolean press) {
		if (press) {
			inputMap |= button;
		} else {
			inputMap &= ~button;
		}
	}
	
	protected abstract void handleAnalog(Component comp, float value);
	protected abstract void handleButtons(Component comp, float value);
}
