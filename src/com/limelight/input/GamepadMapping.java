package com.limelight.input;

import java.io.Serializable;
import java.util.HashMap;
import java.util.Map.Entry;

import net.java.games.input.Component;

public class GamepadMapping implements Serializable {
	private static final long serialVersionUID = -185035113915743149L;
	
	private HashMap<String, ControllerComponent> mapping;
	
	public GamepadMapping() {
		mapping = new HashMap<String, ControllerComponent>();
	}
	
	public void insertMapping(ControllerComponent contComp, Component comp) {
		mapping.put(comp.getIdentifier().getName(), contComp);
	}
	
	public ControllerComponent getControllerComponent(Component comp) {
		return mapping.get(comp.getIdentifier().getName());
	}
	
	public void removeMapping(Component comp) {
		mapping.remove(comp.getIdentifier().getName());
	}
	
	/**
	 * Gets the mapping for the specified component.</br>
	 * NOTE: Use sparingly takes O(N) time.
	 * @param contComp the component to get a mapping for
	 * @return a mapping or an empty string if there is none
	 */
	public String getMapping(ControllerComponent contComp) {
		for (Entry<String, ControllerComponent> entry : mapping.entrySet()) {
			if (entry.getValue().equals(contComp)) {
				return entry.getKey();
			}
		}
		return "";
	}
}
