package com.limelight.input.gamepad;

import java.io.Serializable;
import java.util.HashMap;
import java.util.Map.Entry;


import net.java.games.input.Component;

/**
 * Mappings for gamepad components
 * @author Diego Waxemberg
 */
public class GamepadMapping implements Serializable {
	private static final long serialVersionUID = -185035113915743149L;
	
	private HashMap<String, Mapping> mapping;

	/**
	 * Constructs a new mapping that has nothing mapped.
	 */
	public GamepadMapping() {
		mapping = new HashMap<String, Mapping>();
	}
	
	/**
	 * Inserts the specified mapping into this map
	 * @param toMap a <code>Mapping</code> that will be mapped to the specified gamepad component
	 * @param comp the gamepad component to map to.
	 */
	public void insertMapping(Mapping toMap, SourceComponent comp) {
		// This is the base mapping for components with multiple "buttons"
		mapping.put(comp.getComponentId(), toMap);
		
		// This is the more-specific mapping for the specific buttons
		mapping.put(comp.getFullUniqueId(), toMap);
	}
	
	/**
	 * Gets the mapping for the specified gamepad component
	 * @param comp the gamepad component to get a mapping for
	 * @return a mapping for the requested component
	 */
	public Mapping get(Component comp) {
		return mapping.get(comp.getIdentifier().getName());
	}
	
	/**
	 * Gets the mapping for the specified source component
	 * @param comp the source component to get a mapping for
	 * @return a mapping for the requested component
	 */
	public Mapping get(SourceComponent comp) {
		return mapping.get(comp.getFullUniqueId());
	}
	
	/**
	 * Removes the mapping to the specified component
	 * @param comp the component to no longer be mapped.
	 */
	public void remove(SourceComponent comp) {
		// Remove the most specific mapping
		mapping.remove(comp.getFullUniqueId());
		
		for (Entry<String, Mapping> entry : mapping.entrySet()) {
			if (entry.getKey().startsWith(comp.getComponentId())) {
				return;
			}
		}
		
		// Remove the common mapping if no more specific mappings remain
		mapping.remove(comp.getComponentId());
	}
	
	/**
	 * Gets the mapped ControllerComponent for the specified ControllerComponent.</br>
	 * @param contComp the component to get a mapping for
	 * @return a mapping or an null if there is none
	 */
	public Mapping get(GamepadComponent contComp) {
		//#allTheJank
		for (Entry<String, Mapping> entry : mapping.entrySet()) {
			if (entry.getValue().contComp == contComp) {
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
	public String getMapping(GamepadComponent contComp) {
		for (Entry<String, Mapping> entry : mapping.entrySet()) {
			if (entry.getValue().contComp == contComp) {
				return entry.getKey();
			}
		}
		return "";
	}
	
	/**
	 * Represents a mapping, that is which gamepad component, whether it is inverted, a trigger, etc.
	 * @author Diego Waxemberg
	 */
	public class Mapping implements Serializable {
		private static final long serialVersionUID = -8407172977953214242L;
		
		/**
		 * The component this mapping belongs to
		 */
		public GamepadComponent contComp;
		
		/**
		 * Whether the value of this component should be inverted
		 */
		public boolean invert;
		
		/**
		 * Whether this component should be treated as a trigger
		 */
		public boolean trigger;
		
		/**
		 * Constructs a new mapping with the specified configuration
		 * @param contComp the component this mapping belongs to
		 * @param invert whether the value should be inverted
		 * @param trigger whether this component should be treated as a trigger
		 */
		public Mapping(GamepadComponent contComp, boolean invert, boolean trigger) {
			this.contComp = contComp;
			this.invert = invert;
			this.trigger = trigger;
		}
	}
}
