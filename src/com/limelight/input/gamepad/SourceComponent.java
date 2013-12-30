package com.limelight.input.gamepad;

import net.java.games.input.Component;

public class SourceComponent {
	private Component component;
	private String id;
	
	public SourceComponent(Component component, String id) {
		this.component = component;
		this.id = id;
	}
	
	public Component getComponent() {
		return component;
	}
	
	public String getId() {
		return id;
	}
	
	public String getFullUniqueId() {
		return getComponentId() + " " + getId();
	}
	
	public String getComponentId() {
		return component.getIdentifier().getName();
	}
}
