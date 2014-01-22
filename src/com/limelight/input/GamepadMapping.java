package com.limelight.input;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.lang.reflect.Field;
import java.util.Map;
import java.util.Properties;

/**
 * Mapping between gamepad and gamestream input
 * @author Iwan Timmer
 */
public class GamepadMapping {
	
	public short abs_x = EvdevConstants.ABS_X;
	public short abs_y = EvdevConstants.ABS_Y;
	public short abs_rx = EvdevConstants.ABS_RX;
	public short abs_ry = EvdevConstants.ABS_RY;
	
	public short abs_throttle = EvdevConstants.ABS_THROTTLE;
	public short abs_rudder = EvdevConstants.ABS_RUDDER;
	
	public short btn_south = EvdevConstants.BTN_SOUTH;
	public short btn_east = EvdevConstants.BTN_EAST;
	public short btn_north = EvdevConstants.BTN_NORTH;
	public short btn_west = EvdevConstants.BTN_WEST;
	
	public short btn_select = EvdevConstants.BTN_SELECT;
	public short btn_start = EvdevConstants.BTN_START;
	public short btn_mode = EvdevConstants.BTN_MODE;
	public short btn_thumbl = EvdevConstants.BTN_THUMBL;
	public short btn_thumbr = EvdevConstants.BTN_THUMBR;
	public short btn_tl = EvdevConstants.BTN_TL;
	public short btn_tr = EvdevConstants.BTN_TR;
	
	public short btn_dpad_up = EvdevConstants.BTN_DPAD_UP;
	public short btn_dpad_down = EvdevConstants.BTN_DPAD_DOWN;
	public short btn_dpad_left = EvdevConstants.BTN_DPAD_LEFT;
	public short btn_dpad_right = EvdevConstants.BTN_DPAD_RIGHT;
	
	public GamepadMapping(File file) throws IOException {
		Properties props = new Properties();
		props.load(new FileInputStream(file));
		
		for (Map.Entry entry:props.entrySet()) {
			try {
				Field field = this.getClass().getField(entry.getKey().toString());
				field.setShort(this, Short.parseShort(entry.getValue().toString()));
			} catch (NoSuchFieldException e) {
				System.err.println("No mapping found named " + entry.getKey());
			} catch (NumberFormatException e) {
				System.err.println("Not a number for " + entry.getKey());
			} catch (SecurityException | IllegalArgumentException | IllegalAccessException e) {
				System.err.println("Can't change mapping for " + entry.getKey());
			}
		}
	}
	
	public GamepadMapping() {
	}
	
}
