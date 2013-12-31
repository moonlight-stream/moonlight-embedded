package com.limelight.input.gamepad;

import java.io.Serializable;
import java.util.HashMap;
import java.util.Map.Entry;

/**
 * Mappings for gamepad components
 * @author Diego Waxemberg
 */
public class GamepadMapping implements Serializable {
	private static final long serialVersionUID = -185035113915743149L;
	
	private HashMap<SourceComponent, Mapping> mapping;

	/**
	 * Constructs a new mapping that has nothing mapped.
	 */
	public GamepadMapping() {
		mapping = new HashMap<SourceComponent, Mapping>();
	}
	
	/**
	 * Inserts the specified mapping into this map
	 * @param toMap a <code>Mapping</code> that will be mapped to the specified gamepad component
	 * @param comp the gamepad component to map to.
	 */
	public void insertMapping(Mapping toMap, SourceComponent comp) {
		mapping.put(comp, toMap);
	}
	
	/**
	 * Gets the mapping for the specified gamepad component
	 * @param comp the gamepad component to get a mapping for
	 * @return a mapping for the requested component
	 */
	public Mapping get(SourceComponent comp) {
		return mapping.get(comp);
	}
	
	/**
	 * Removes the mapping to the specified component
	 * @param comp the component to no longer be mapped.
	 */
	public void remove(SourceComponent comp) {
		mapping.remove(comp);
	}
	
	/**
	 * Gets the mapped ControllerComponent for the specified ControllerComponent.</br>
	 * <b>NOTE: Iterates a hashmap, use sparingly</b>
	 * @param padComp the component to get a mapping for
	 * @return a mapping or an null if there is none
	 */
	public Mapping get(GamepadComponent padComp) {
		//#allTheJank
		for (Entry<SourceComponent, Mapping> entry : mapping.entrySet()) {
			if (entry.getValue().padComp == padComp) {
				return entry.getValue();
			}
		}
		return null;
	}
	
	/**
	 * Gets the mapping for the specified component.</br>
	 * <b>NOTE: Iterates a hashmap, use sparingly</b>
	 * @param padComp the component to get a mapping for
	 * @return a mapping or an empty string if there is none
	 */
	public SourceComponent getMapping(GamepadComponent padComp) {
		for (Entry<SourceComponent, Mapping> entry : mapping.entrySet()) {
			if (entry.getValue().padComp == padComp) {
				return entry.getKey();
			}
		}
		return null;
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
		public GamepadComponent padComp;
		
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
		 * @param padComp the component this mapping belongs to
		 * @param invert whether the value should be inverted
		 * @param trigger whether this component should be treated as a trigger
		 */
		public Mapping(GamepadComponent padComp, boolean invert, boolean trigger) {
			this.padComp = padComp;
			this.invert = invert;
			this.trigger = trigger;
		}
	}
}
