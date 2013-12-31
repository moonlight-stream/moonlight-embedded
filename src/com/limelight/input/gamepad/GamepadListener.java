package com.limelight.input.gamepad;

import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;

import com.limelight.input.Device;
import com.limelight.input.DeviceListener;

/**
 * Listens to <code>Controller</code>s connected to this computer and gives any gamepad to the gamepad handler
 * @author Diego Waxemberg
 */
public class GamepadListener implements NativeGamepadListener {
	private HashMap<Integer, Device> devices;
	private List<DeviceListener> listeners;
	
	private static GamepadListener singleton;
	
	public static GamepadListener getInstance() {
		if (singleton == null) {
			singleton = new GamepadListener();
		}
		return singleton;
	}
	
	private GamepadListener() {
		devices = new HashMap<Integer, Device>();
		listeners = new LinkedList<DeviceListener>();
	}
	
	public int deviceCount() {
		return devices.size();
	}
	
	public void addDeviceListener(DeviceListener listener) {
		listeners.add(listener);
	}
	
	public List<DeviceListener> getListeners() {
		return Collections.unmodifiableList(listeners);
	}
	
	public void removeListener(DeviceListener listener) {
		listeners.remove(listener);
	}
	
	@Override
	public void deviceAttached(int deviceId, int numButtons, int numAxes) {
		devices.put(deviceId, new Device(deviceId, numButtons, numAxes));
	}

	@Override
	public void deviceRemoved(int deviceId) {
		devices.remove(deviceId);
	}

	@Override
	public void buttonDown(int deviceId, int buttonId) {
		Device dev = devices.get(deviceId);
		for (DeviceListener listener : listeners) {
			listener.handleButton(dev, buttonId, true);
		}
	}

	@Override
	public void buttonUp(int deviceId, int buttonId) {
		Device dev = devices.get(deviceId);
		for (DeviceListener listener : listeners) {
			listener.handleButton(dev, buttonId, false);
		}
	}

	@Override
	public void axisMoved(int deviceId, int axisId, float value, float lastValue) {
		Device dev = devices.get(deviceId);
		for (DeviceListener listener : listeners) {
			listener.handleAxis(dev, axisId, value, lastValue);
		}
	}

}
