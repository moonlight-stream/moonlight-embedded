package com.limelight.input;

import com.limelight.nvstream.NvConnection;
import com.limelight.nvstream.input.ControllerPacket;
import com.limelight.nvstream.input.KeyboardPacket;
import com.limelight.nvstream.input.MouseButtonPacket;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.FileChannel;

/**
 * Class that handles keyboard input using the Evdev interface
 * @author Iwan Timmer
 */
public class EvdevHandler implements Runnable {

	
	private static KeyboardTranslator translator;
	
	/* GFE's prefix for every key code */
	public static final short KEY_PREFIX = (short) 0x80;
	
	/* Gamepad state */
	private short buttonFlags;
	private byte leftTrigger, rightTrigger;
	private short leftStickX, leftStickY, rightStickX, rightStickY;
	
	private EvdevAbsolute absLX, absLY, absRX, absRY, absLT, absRT, absDX, absDY;
	
	private NvConnection conn;
	private FileChannel deviceInput;
	private ByteBuffer inputBuffer;
	private boolean loggingInput;
	private GamepadMapping mapping;
	
	public EvdevHandler(NvConnection conn, String device, GamepadMapping mapping, boolean loggingInput) throws FileNotFoundException, IOException {
		this.conn = conn;
		this.mapping = mapping;
		this.loggingInput = loggingInput;
		File file = new File(device);
		if (!file.exists())
			throw new FileNotFoundException("File " + device + " not found");
		if (!file.canRead())
			throw new IOException("Can't read from " + device);
			
		FileInputStream in = new FileInputStream(file);
        deviceInput = in.getChannel();
		inputBuffer = ByteBuffer.allocate(EvdevConstants.MAX_STRUCT_SIZE_BYTES);
		inputBuffer.order(ByteOrder.nativeOrder());
		
		absLX = new EvdevAbsolute(device, mapping.abs_x, mapping.reverse_x);
		absLY = new EvdevAbsolute(device, mapping.abs_y, !mapping.reverse_y);
		absRX = new EvdevAbsolute(device, mapping.abs_rx, mapping.reverse_rx);
		absRY = new EvdevAbsolute(device, mapping.abs_ry, !mapping.reverse_ry);
		absLT = new EvdevAbsolute(device, mapping.abs_rudder, mapping.reverse_rudder);
		absRT = new EvdevAbsolute(device, mapping.abs_throttle, mapping.reverse_throttle);
		absDX = new EvdevAbsolute(device, mapping.abs_dpad_x, mapping.reverse_dpad_x);
		absDY = new EvdevAbsolute(device, mapping.abs_dpad_y, mapping.reverse_dpad_y);
		
		translator = new KeyboardTranslator(conn);
	}

	private void logInput(String msg) {
		if (loggingInput)
			System.out.println(msg);
	}

	public void start() {
		Thread thread = new Thread(this);
		thread.setDaemon(true);
		thread.setName("Input - Receiver");
		thread.start();
	}
	
	private void parseEvent(ByteBuffer buffer) {
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
		
		if (type==EvdevConstants.EV_KEY) {
			if (code<EvdevConstants.KEY_CODES.length) {
				short gfCode = translator.translate(EvdevConstants.KEY_CODES[code]);

				if (value==EvdevConstants.KEY_PRESSED)
					conn.sendKeyboardInput(gfCode, KeyboardPacket.KEY_DOWN, (byte) 0);
				else if (value==EvdevConstants.KEY_RELEASED)
					conn.sendKeyboardInput(gfCode, KeyboardPacket.KEY_UP, (byte) 0);
			} else {
				byte mouseButton = 0;
				short gamepadButton = 0;
				
				if (code==EvdevConstants.BTN_LEFT)
					mouseButton = MouseButtonPacket.BUTTON_1;
				else if (code==EvdevConstants.BTN_MIDDLE)
					mouseButton = MouseButtonPacket.BUTTON_2;
				else if (code==EvdevConstants.BTN_RIGHT)
					mouseButton = MouseButtonPacket.BUTTON_3;
				else if (code==mapping.btn_south) {
					gamepadButton = ControllerPacket.A_FLAG;
					logInput("A(South)");
				} else if (code==mapping.btn_west) {
					gamepadButton = ControllerPacket.X_FLAG;
					logInput("X(West)");
				} else if (code==mapping.btn_north) {
					gamepadButton = ControllerPacket.Y_FLAG;
					logInput("Y(North)");
				} else if (code==mapping.btn_east) {
					gamepadButton = ControllerPacket.B_FLAG;
					logInput("B(East)");
				} else if (code==mapping.btn_dpad_up) {
					gamepadButton = ControllerPacket.UP_FLAG;
					logInput("D-UP");
				} else if (code==mapping.btn_dpad_down) {
					gamepadButton = ControllerPacket.DOWN_FLAG;
					logInput("D-DOWN");
				} else if (code==mapping.btn_dpad_left) {
					gamepadButton = ControllerPacket.LEFT_FLAG;
					logInput("D-LEFT");
				} else if (code==mapping.btn_dpad_right) {
					gamepadButton = ControllerPacket.RIGHT_FLAG;
					logInput("D-RIGHT");
				} else if (code==mapping.btn_thumbl) {
					gamepadButton = ControllerPacket.LS_CLK_FLAG;
					logInput("Left Thumb");
				} else if (code==mapping.btn_thumbr) {
					gamepadButton = ControllerPacket.RS_CLK_FLAG;
					logInput("Right Thumb");
				} else if (code==mapping.btn_tl) {
					gamepadButton = ControllerPacket.LB_FLAG;
					logInput("Left Shoulder");
				} else if (code==mapping.btn_tr) {
					gamepadButton = ControllerPacket.RB_FLAG;
					logInput("Right Shoulder");
				} else if (code==mapping.btn_start) {
					gamepadButton = ControllerPacket.PLAY_FLAG;
					logInput("Play");
				} else if (code==mapping.btn_select) {
					gamepadButton = ControllerPacket.BACK_FLAG;
					logInput("Back");
				} else if (code==mapping.btn_mode) {
					gamepadButton = ControllerPacket.SPECIAL_BUTTON_FLAG;
					logInput("Mode");
				}
			
				if (mouseButton>0) {
					if (value==EvdevConstants.KEY_PRESSED)
						conn.sendMouseButtonDown(mouseButton);
					else if (value==EvdevConstants.KEY_RELEASED)
						conn.sendMouseButtonUp(mouseButton);
				} else {
					if (gamepadButton != 0) {
						if (value==EvdevConstants.KEY_PRESSED) {
							buttonFlags |= gamepadButton;
						} else  if (value==EvdevConstants.KEY_RELEASED){
							buttonFlags &= ~gamepadButton;
						}
					} else if (code==mapping.btn_throttle) {
						leftTrigger = (byte) (value==EvdevConstants.KEY_PRESSED ? -1 : 0);
						logInput("LeftTrigger-As-Button (Throttle): " + leftTrigger);
					} else if (code==mapping.btn_rudder) {
						rightTrigger = (byte) (value==EvdevConstants.KEY_PRESSED ? -1 : 0);
						logInput("RightTrigger-As-Button (Rudder): " + rightTrigger);
					} else {
						logInput("Unrecognized Code: " + code);
						return;
					}
					conn.sendControllerInput(buttonFlags, leftTrigger, rightTrigger, leftStickX, leftStickY, rightStickX, rightStickY);
				}
			}
		} else if (type==EvdevConstants.EV_REL) {
			if (code==EvdevConstants.REL_X)
				conn.sendMouseMove((short) value, (short) 0);
			else if (code==EvdevConstants.REL_Y)
				conn.sendMouseMove((short) 0, (short) value);
		} else if (type==EvdevConstants.EV_ABS) {
			if (code==mapping.abs_x) {
				leftStickX = accountForDeadzone(absLX.getShort(value));
				logInput("Left Stick X:" + leftStickX);
			} else if (code==mapping.abs_y) {
				leftStickY = accountForDeadzone(absLY.getShort(value));
				logInput("Left Stick Y:" + leftStickY);
			} else if (code==mapping.abs_rx) {
				rightStickX = accountForDeadzone(absRX.getShort(value));
				logInput("Right Stick X:" + rightStickX);
			} else if (code==mapping.abs_ry) {
				rightStickY = accountForDeadzone(absRY.getShort(value));
				logInput("Right Stick Y:" + rightStickY);
			} else if (code==mapping.abs_throttle) {
				leftTrigger = absLT.getByte(value);
				logInput("LeftTrigger (Throttle): " + leftTrigger);
			} else if (code==mapping.abs_rudder) {
				rightTrigger = absRT.getByte(value);
				logInput("RightTrigger (Rudder): " + rightTrigger);
			} else if (code==mapping.abs_dpad_x) {
				int dir = absRT.getDirection(value);
				if (dir==EvdevAbsolute.UP) {
					logInput("D-Pad X UP");
					buttonFlags |= ControllerPacket.RIGHT_FLAG;
					buttonFlags &= ~ControllerPacket.LEFT_FLAG;
				} else if (dir==EvdevAbsolute.NONE) {
					logInput("D-Pad X NONE");
					buttonFlags &= ~ControllerPacket.LEFT_FLAG;
					buttonFlags &= ~ControllerPacket.RIGHT_FLAG;
				} else if (dir==EvdevAbsolute.DOWN) {
					logInput("D-Pad X DOWN");
					buttonFlags |= ControllerPacket.LEFT_FLAG;
					buttonFlags &= ~ControllerPacket.RIGHT_FLAG;
				}
			} else if (code==mapping.abs_dpad_y) {
				int dir = absRT.getDirection(value);
				if (dir==EvdevAbsolute.DOWN) {
					logInput("D-Pad Y DOWN");
					buttonFlags |= ControllerPacket.UP_FLAG;
					buttonFlags &= ~ControllerPacket.DOWN_FLAG;
				} else if (dir==EvdevAbsolute.NONE) {
					logInput("D-Pad Y NONE");
					buttonFlags &= ~ControllerPacket.DOWN_FLAG;
					buttonFlags &= ~ControllerPacket.UP_FLAG;
				} else if (dir==EvdevAbsolute.UP) {
					logInput("D-Pad Y UP");
					buttonFlags |= ControllerPacket.DOWN_FLAG;
					buttonFlags &= ~ControllerPacket.UP_FLAG;
				}
			} else {
				logInput("Unrecognized [Code,Value]: [" + code + "," + value + "]");
			}

			conn.sendControllerInput(buttonFlags, leftTrigger, rightTrigger, leftStickX, leftStickY, rightStickX, rightStickY);
		}
	}

	private short accountForDeadzone(short value) {
		if (Math.abs(value) > mapping.abs_deadzone) {
			return value;
		} else {
			return 0;
		}
	}

	@Override
	public void run() {
		try {
			while (true) {
				while(inputBuffer.remaining()==EvdevConstants.MAX_STRUCT_SIZE_BYTES)
					deviceInput.read(inputBuffer);

				inputBuffer.flip();
				parseEvent(inputBuffer);
				inputBuffer.clear();
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
}
