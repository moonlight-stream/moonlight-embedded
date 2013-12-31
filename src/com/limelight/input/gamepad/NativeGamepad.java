package com.limelight.input.gamepad;

import java.util.ArrayList;

public class NativeGamepad {
	public static final int DEFAULT_DEVICE_POLLING_INTERVAL = 1000;
	public static final int DEFAULT_EVENT_POLLING_INTERVAL = 20;
	
	private static ArrayList<NativeGamepadListener> listenerList =
			new ArrayList<NativeGamepadListener>();
	private static boolean running = false;
	private static Thread deviceThread = null;
	private static Thread eventThread = null;
	private static int devicePollingIntervalMs = DEFAULT_DEVICE_POLLING_INTERVAL;
	private static int eventPollingIntervalMs = DEFAULT_EVENT_POLLING_INTERVAL;

	static {
		System.loadLibrary("gamepad_jni");
		
		NativeGamepad.init();
	}
	
	private static native void init();
	private static native void shutdown();
	private static native int numDevices();
	private static native void detectDevices();
	private static native void processEvents();
	
	public static void addListener(NativeGamepadListener listener) {
		listenerList.add(listener);
	}
	
	public static void removeListener(NativeGamepadListener listener) {
		listenerList.remove(listener);
	}
	
	public static boolean isRunning() {
		return running;
	}
	
	public static void setDevicePollingInterval(int interval) {
		devicePollingIntervalMs = interval;
	}
	
	public static int getDevicePollingInterval() {
		return devicePollingIntervalMs;
	}
	
	public static void setEventPollingInterval(int interval) {
		eventPollingIntervalMs = interval;
	}
	
	public static int getEventPollingInterval() {
		return eventPollingIntervalMs;
	}
	
	public static void start() {
		if (!running) {
			startDevicePolling();
			startEventPolling();
			running = true;
		}
	}
	
	public static void stop() {
		if (running) {
			stopEventPolling();
			stopDevicePolling();
			running = false;
		}
	}
	
	public static void release() {
		NativeGamepad.shutdown();
	}
	
	public static int getDeviceCount() {
		if (!running) {
			throw new IllegalStateException("NativeGamepad not running");
		}
		
		return NativeGamepad.numDevices();
	}
	
	private static void startDevicePolling() {
		deviceThread = new Thread() {
			@Override
			public void run() {
				while (!isInterrupted()) {
					NativeGamepad.detectDevices();
					
					try {
						Thread.sleep(devicePollingIntervalMs);
					} catch (InterruptedException e) {
						return;
					}
				}
			}
		};
		deviceThread.setName("Native Gamepad - Device Polling Thread");
		deviceThread.start();
	}
	
	private static void startEventPolling() {
		eventThread = new Thread() {
			@Override
			public void run() {
				while (!isInterrupted()) {
					NativeGamepad.processEvents();
					
					try {
						Thread.sleep(eventPollingIntervalMs);
					} catch (InterruptedException e) {
						return;
					}
				}
			}
		};
		eventThread.setName("Native Gamepad - Event Polling Thread");
		eventThread.start();
	}
	
	private static void stopDevicePolling() {
		if (deviceThread != null) {
			deviceThread.interrupt();
			
			try {
				deviceThread.join();
			} catch (InterruptedException e) {}
		}
	}
	
	private static void stopEventPolling() {
		if (eventThread != null) {
			eventThread.interrupt();
			
			try {
				eventThread.join();
			} catch (InterruptedException e) {}
		}
	}
	
	public static void deviceAttachCallback(int deviceId, int numButtons, int numAxes) {
		for (NativeGamepadListener listener : listenerList) {
			listener.deviceAttached(deviceId, numButtons, numAxes);
		}
	}
	
	public static void deviceRemoveCallback(int deviceId) {
		for (NativeGamepadListener listener : listenerList) {
			listener.deviceRemoved(deviceId);
		}
	}
	
	public static void buttonUpCallback(int deviceId, int buttonId) {
		for (NativeGamepadListener listener : listenerList) {
			listener.buttonUp(deviceId, buttonId);
		}
	}
	
	public static void buttonDownCallback(int deviceId, int buttonId) {
		for (NativeGamepadListener listener : listenerList) {
			listener.buttonDown(deviceId, buttonId);
		}
	}
	
	public static void axisMovedCallback(int deviceId, int axisId, float value, float lastValue) {
		for (NativeGamepadListener listener : listenerList) {
			listener.axisMoved(deviceId, axisId, value, lastValue);
		}
	}
}
