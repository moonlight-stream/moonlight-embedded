package com.limelight.input;

import com.limelight.nvstream.NvConnection;
import com.limelight.nvstream.input.KeyboardPacket;
import com.limelight.nvstream.input.MouseButtonPacket;
import java.awt.event.KeyEvent;
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
	
	private static final short KEY_CODES[] = {
		0, //KeyEvent.VK_RESERVED
		KeyEvent.VK_ESCAPE,
		KeyEvent.VK_1,
		KeyEvent.VK_2,
		KeyEvent.VK_3,
		KeyEvent.VK_4,
		KeyEvent.VK_5,
		KeyEvent.VK_6,
		KeyEvent.VK_7,
		KeyEvent.VK_8,
		KeyEvent.VK_9,
		KeyEvent.VK_0,
		KeyEvent.VK_MINUS,
		KeyEvent.VK_EQUALS,
		KeyEvent.VK_BACK_SPACE,
		KeyEvent.VK_TAB,
		KeyEvent.VK_Q,
		KeyEvent.VK_W,
		KeyEvent.VK_E,
		KeyEvent.VK_R,
		KeyEvent.VK_T,
		KeyEvent.VK_Y,
		KeyEvent.VK_U,
		KeyEvent.VK_I,
		KeyEvent.VK_O,
		KeyEvent.VK_P,
		0, //KeyEvent.VK_LEFTBRACE,
		0, //KeyEvent.VK_RIGHTBRACE,
		KeyEvent.VK_ENTER,
		KeyEvent.VK_CONTROL, // Left control */
		KeyEvent.VK_A,
		KeyEvent.VK_S,
		KeyEvent.VK_D,
		KeyEvent.VK_F,
		KeyEvent.VK_G,
		KeyEvent.VK_H,
		KeyEvent.VK_J,
		KeyEvent.VK_K,
		KeyEvent.VK_L,
		KeyEvent.VK_SEMICOLON,
		0, //KeyEvent.VK_APOSTROPHE,
		0, //KeyEvent.VK_GRAVE,
		0, //KeyEvent.VK_LEFTSHIFT,
		0, //KeyEvent.VK_BACKSLASH,
		KeyEvent.VK_Z,
		KeyEvent.VK_X,
		KeyEvent.VK_C,
		KeyEvent.VK_V,
		KeyEvent.VK_B,
		KeyEvent.VK_N,
		KeyEvent.VK_M,
		KeyEvent.VK_COMMA,
		0, //KeyEvent.VK_DOT,
		KeyEvent.VK_SLASH,
		0, //KeyEvent.VK_RIGHTSHIFT,
		0, //KeyEvent.VK_KPASTERISK,
		0, //KeyEvent.VK_LEFTALT,
		KeyEvent.VK_SPACE,
		0, //KeyEvent.VK_CAPSLOCK,
		KeyEvent.VK_F1,
		KeyEvent.VK_F2,
		KeyEvent.VK_F3,
		KeyEvent.VK_F4,
		KeyEvent.VK_F5,
		KeyEvent.VK_F6,
		KeyEvent.VK_F7,
		KeyEvent.VK_F8,
		KeyEvent.VK_F9,
		KeyEvent.VK_F10,
		KeyEvent.VK_NUM_LOCK,
		KeyEvent.VK_SCROLL_LOCK,
		KeyEvent.VK_NUMPAD7,
		KeyEvent.VK_NUMPAD8,
		KeyEvent.VK_NUMPAD9,
		0, //KeyEvent.VK_NUMPAD_MINUS,
		KeyEvent.VK_NUMPAD4,
		KeyEvent.VK_NUMPAD5,
		KeyEvent.VK_NUMPAD6,
		0, //KeyEvent.VK_NUMPADPLUS,
		KeyEvent.VK_NUMPAD1,
		KeyEvent.VK_NUMPAD2,
		KeyEvent.VK_NUMPAD3,
		KeyEvent.VK_NUMPAD0,
		0, //KeyEvent.VK_NUMPADDOT,
		0,
		0, //KeyEvent.VK_ZENKAKUHANKAKU,
		0, //KeyEvent.VK_102ND,
		KeyEvent.VK_F11,
		KeyEvent.VK_F12,
		0, //KeyEvent.VK_RO,
		KeyEvent.VK_KATAKANA,
		KeyEvent.VK_HIRAGANA,
		0, //KeyEvent.VK_HENKAN,
		0, //KeyEvent.VK_KATAKANAHIRAGANA,
		0, //KeyEvent.VK_MUHENKAN,
		0, //KeyEvent.VK_KPJPCOMMA,
		0, //KeyEvent.VK_KPENTER,
		0, //KeyEvent.VK_RIGHTCTRL,
		0, //KeyEvent.VK_KPSLASH,
		0, //KeyEvent.VK_SYSRQ,
		0, //KeyEvent.VK_RIGHTALT,
		0, //KeyEvent.VK_LINEFEED,
		KeyEvent.VK_HOME,
		KeyEvent.VK_UP,
		KeyEvent.VK_PAGE_UP,
		KeyEvent.VK_LEFT,
		KeyEvent.VK_RIGHT,
		KeyEvent.VK_END,
		KeyEvent.VK_DOWN,
		KeyEvent.VK_PAGE_DOWN,
		KeyEvent.VK_INSERT,
		KeyEvent.VK_DELETE,
		0, //KeyEvent.VK_MACRO,
		0, //KeyEvent.VK_MUTE,
		0, //KeyEvent.VK_VOLUMEDOWN,
		0, //KeyEvent.VK_VOLUMEUP,
		0, //KeyEvent.VK_POWER, /* SC System Power Down */
		0, //KeyEvent.VK_KPEQUAL,
		0, //KeyEvent.VK_KPPLUSMINUS,
		KeyEvent.VK_PAUSE,
		0, //KeyEvent.VK_SCALE, /* AL Compiz Scale (Expose) */
	};

	private static final int MAX_STRUCT_SIZE_BYTES = 24;
	
	/* GFE's prefix for every key code */
	public static final short KEY_PREFIX = (short) 0x80;
	
	/* Event types */
	public static final short EV_KEY = 0x01;
	public static final short EV_REL = 0x02;
	
	public static final short REL_X = 0x00;
	public static final short REL_Y = 0x01;
	
	public static final short BTN_LEFT = 0x110;
	public static final short BTN_RIGHT = 0x111;
	public static final short BTN_MIDDLE = 0x112;
	
	/* Events */
	public static final int KEY_RELEASED = 0;
	public static final int KEY_PRESSED = 1;
	
	private NvConnection conn;
	private FileChannel deviceInput;
	private ByteBuffer inputBuffer;
	
	public EvdevHandler(NvConnection conn, String device) throws FileNotFoundException {
		this.conn = conn;
		FileInputStream in = new FileInputStream(device);
        deviceInput = in.getChannel();
		inputBuffer = ByteBuffer.allocate(MAX_STRUCT_SIZE_BYTES);
		inputBuffer.order(ByteOrder.nativeOrder());
		
		translator = new KeyboardTranslator(conn);
	}
	
	public void start() {
		Thread thread = new Thread(this);
		thread.setDaemon(true);
		thread.setName("Input - Receiver");
		thread.start();
	}
	
	private void parseEvent(ByteBuffer buffer) {
		if (buffer.limit()==MAX_STRUCT_SIZE_BYTES) {
			long time_sec = buffer.getLong();
			long time_usec = buffer.getLong();
		} else {
			int time_sec = buffer.getInt();
			int time_usec = buffer.getInt();
		}
		short type = buffer.getShort();
		short code = buffer.getShort();
		int value = buffer.getInt();
		
		if (type==EV_KEY) {
			if (code<KEY_CODES.length) {
				short gfCode = translator.translate(KEY_CODES[code]);

				if (value==KEY_PRESSED)
					conn.sendKeyboardInput(gfCode, KeyboardPacket.KEY_DOWN, (byte) 0);
				else if (value==KEY_RELEASED)
					conn.sendKeyboardInput(gfCode, KeyboardPacket.KEY_UP, (byte) 0);
			} else {
				byte mouseButton = 0;
				
				switch (code) {
					case BTN_LEFT:
						mouseButton = MouseButtonPacket.BUTTON_1;
						break;
					case BTN_MIDDLE:
						mouseButton = MouseButtonPacket.BUTTON_2;
						break;
					case BTN_RIGHT:
						mouseButton = MouseButtonPacket.BUTTON_3;
						break;
				}
				
				if (mouseButton>0) {
					if (value==KEY_PRESSED)
						conn.sendMouseButtonDown(mouseButton);
					else if (value==KEY_RELEASED)
						conn.sendMouseButtonUp(mouseButton);						
				}
			}
		} else if (type==EV_REL) {
			switch (code) {
				case REL_X:
					conn.sendMouseMove((short) value, (short) 0);
					break;
				case REL_Y:
					conn.sendMouseMove((short) 0, (short) value);
					break;
			}
		}
	}

	@Override
	public void run() {
		try {
			while (true) {
				while(inputBuffer.remaining()==MAX_STRUCT_SIZE_BYTES)
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
