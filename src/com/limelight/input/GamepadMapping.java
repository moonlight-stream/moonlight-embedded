package com.limelight.input;

import java.io.Serializable;
import java.util.HashMap;
import java.util.Map.Entry;

import net.java.games.input.Component;

public class GamepadMapping implements Serializable {
	private static final long serialVersionUID = -185035113915743149L;
	
	private HashMap<String, Mapping> mapping;

	public GamepadMapping() {
		mapping = new HashMap<String, Mapping>();
	}
	
	public void insertMapping(Mapping toMap, Component comp) {
		mapping.put(comp.getIdentifier().getName(), toMap);
	}
	
	public Mapping get(Component comp) {
		return mapping.get(comp.getIdentifier().getName());
	}
	
	public void remove(Component comp) {
		mapping.remove(comp.getIdentifier().getName());
	}
	
	/**
	 * Gets the mapped ControllerComponent for the specified ControllerComponent.</br>
	 * @param contComp the component to get a mapping for
	 * @return a mapping or an null if there is none
	 */
	public Mapping get(ControllerComponent contComp) {
		//#allTheJank
		for (Entry<String, Mapping> entry : mapping.entrySet()) {
			if (entry.getValue().contComp.sameAs(contComp)) {
				return entry.getValue();
			}
		}
		return null;
	}
	
	/**
	 * Gets the mapping for the specified component.</br>
	 * @param contComp the component to get a mapping for
	 * @return a mapping or an empty string if there is none
	 */
	public String getMapping(ControllerComponent contComp) {
		for (Entry<String, Mapping> entry : mapping.entrySet()) {
			if (entry.getValue().contComp.sameAs(contComp)) {
				return entry.getKey();
			}
		}
		return "";
	}
	
	public class Mapping implements Serializable {
		private static final long serialVersionUID = -8407172977953214242L;
		
		public ControllerComponent contComp;
		public boolean invert;
		public boolean trigger;
		
		public Mapping(ControllerComponent contComp, boolean invert, boolean trigger) {
			this.contComp = contComp;
			this.invert = invert;
			this.trigger = trigger;
		}
	}
}
