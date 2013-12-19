package com.limelight.input;

import java.util.HashMap;

import net.java.games.input.Component;

public class GamepadSettings {
	private HashMap<ControllerComponent, Component> mapping;
	private HashMap<Component, ControllerComponent> inverseMapping;
	
	public void insertSetting(ControllerComponent contComp, Component comp) {
		mapping.put(contComp, comp);
		inverseMapping.put(comp, contComp);
	}
	
	public Component getComponent(ControllerComponent contComp) {
		return mapping.get(contComp);
	}
	
	public ControllerComponent getControllerComponent(Component comp) {
		return inverseMapping.get(comp);
	}
}
