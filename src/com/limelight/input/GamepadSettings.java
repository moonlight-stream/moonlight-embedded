package com.limelight.input;

import java.io.Serializable;
import java.util.HashMap;

import net.java.games.input.Component;

public class GamepadSettings implements Serializable {
	private static final long serialVersionUID = -185035113915743149L;
	
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
