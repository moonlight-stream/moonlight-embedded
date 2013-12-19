package com.limelight.input;

import java.util.HashMap;

import net.java.games.input.Component;

public class GamepadSettings {
	private HashMap<ControllerComponent, Component> mapping;
	
	public void insertSetting(ControllerComponent contComp, Component comp) {
		mapping.put(contComp, comp);
	}
	
	public Component getComponent(ControllerComponent contComp) {
		return mapping.get(contComp);
	}
}
