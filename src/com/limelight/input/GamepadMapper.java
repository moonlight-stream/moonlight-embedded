package com.limelight.input;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

public class GamepadMapper extends EvdevReader {
	
	private Properties props;
	private Map<Short, EvdevAbsolute> absolutes;
	private String device;
	
	private String key;
	private String abs;
	private boolean dir;
	
	private short current;
	private boolean reverse;
	private boolean lastKey;

	public GamepadMapper(String device) throws FileNotFoundException, IOException {
		super(device);
		this.device = device;
		props = new Properties();
		absolutes = new HashMap<>();
		current = -1;
	}

	@Override
	protected synchronized void parseEvent(ByteBuffer buffer) {
		if (buffer.limit()==EvdevConstants.MAX_STRUCT_SIZE_BYTES) {
			long time_sec = buffer.getLong();
			long time_usec = buffer.getLong();
		} else {
			int time_sec = buffer.getInt();
			int time_usec = buffer.getInt();
		}
		short type = buffer.getShort();
		short code = buffer.getShort();
		int value = buffer.getInt();
		
		boolean set = false;
		if (key != null && type == EvdevConstants.EV_KEY && value == EvdevConstants.KEY_RELEASED) {
			props.put("btn_"+key, Short.toString(code));
			lastKey = true;
			set = true;
		} else if (abs != null && type == EvdevConstants.EV_ABS) {
			if (!absolutes.containsKey(code))
				absolutes.put(code, new EvdevAbsolute(device, code, false));
			
			int val = absolutes.get(code).getShort(value);
			if (val > Short.MAX_VALUE * 0.75) {
				current = code;
				reverse = false;
			} else if (dir && val < Short.MIN_VALUE * 0.75) {
				current = code;
				reverse = true;
			} else if (current != -1 && code == current && val < Short.MAX_VALUE/4) {
				props.put("abs_"+abs, Short.toString(code));
				props.put("revers_"+abs, Boolean.toString(reverse));
				lastKey = false;
				set = true;
			}
		}
		
		if (set) {
			key = null;
			abs = null;
			current = -1;
			notify();		
		}
	}
	
	public synchronized void readKey(String key, String name) throws InterruptedException {
		System.out.println(name);
		this.key = key;
		wait();
	}
	
	public synchronized void readAbs(String abs, String name) throws InterruptedException {
		System.out.println(name);
		this.abs = abs;
		dir = true;
		wait();
	}
	
	public synchronized void readAbsKey(String abs, String key, String name, boolean dir) throws InterruptedException {
		System.out.println("Read " + name);
		this.key = key;
		this.abs = abs;
		this.dir = dir;
		wait();
	}
	
	public void setup() throws InterruptedException {
		start();
		readAbs("x", "Left Stick Right");
		readAbs("y", "Left Stick Down");
		readKey("thumbl", "Left Stick Button");
		
		readAbs("rx", "Right Stick Right");
		readAbs("ry", "Right Stick Down");
		readKey("thumbr", "Right Stick Button");
		
		readAbsKey("dpad_x", "dpad_right", "D-Pad Right", true);
		if (lastKey)
			readKey("dpad_left", "D-Pad Left");
		
		readAbsKey("dpad_y", "dpad_down", "D-Pad Down", true);
		if (lastKey)
			readKey("dpad_up", "D-Pad Up");
		
		readKey("south", "Button 1 (A)");
		readKey("east", "Button 2 (X)");
		readKey("north", "Button 3 (Y)");
		readKey("west", "Button 3 (B)");
		readKey("select", "Back Button");
		readKey("start", "Start Button");
		readKey("mode", "Special Button");
		
		readAbsKey("z", "tl", "Left Trigger", false);
		readAbsKey("rz", "tr", "Right Trigger", false);
		
		readKey("tl2", "Left Bumper");
		readKey("tr2", "Right Bumper");
		
	}
	
	public void save(File file) throws FileNotFoundException, IOException {
		props.store(new FileOutputStream(file), "Gamepad");
	}	
}
