package com.limelight.input.gamepad;

public interface NativeGamepadListener {
	public void deviceAttached(int deviceId, int numButtons, int numAxes);
	
	public void deviceRemoved(int deviceId);
	
	public void buttonDown(int deviceId, int buttonId);
	
	public void buttonUp(int deviceId, int buttonId);
	
	public void axisMoved(int deviceId, int axisId, float value, float lastValue);
}
