package com.limelight.input;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * Class that handles absolute input ranges from evdev device
 * @author Iwan Timmer
 */
public class EvdevAbsolute {
	
	public final static int UP = 1, DOWN = -1, NONE = 0;
	
	private final static int ABS_OFFSET = 0x40;
	private final static int READ_ONLY = 2;
	private final static char EVDEV_TYPE = 'E';
	
	private int avg;
	private int range;
	private int flat;
	
	private boolean reverse;
	
	public EvdevAbsolute(String filename, int axis, boolean reverse) {
		ByteBuffer buffer = ByteBuffer.allocate(6*4);
		buffer.order(ByteOrder.nativeOrder());
		byte[] data = buffer.array();
		int request = getRequest(READ_ONLY, EVDEV_TYPE, ABS_OFFSET+axis, 6*4);
		IO.ioctl(filename, data, request);
		
		buffer.getInt(); //Skip current value
		int min = buffer.getInt();
		int max = buffer.getInt();
		buffer.getInt(); //Skip fuzz
		flat = buffer.getInt();
		avg = (min+max)/2;
		range = max-avg;
		
		this.reverse = reverse;
	}
	
	private int getRequest(int dir, int type, int nr, int size) {
		return (dir << 30) | (size << 16) | (type << 8) | nr;
	}
	
	/**
	 * Convert input value to short range
	 * @param value received input
	 * @return input value as short
	 */
	public short getShort(int value) {
		if (Math.abs(value-avg)<flat)
			return 0;
		else if (value>avg+range)
			return reverse?Short.MIN_VALUE:Short.MAX_VALUE;
		else if (value<avg-range)
			return reverse?Short.MAX_VALUE:Short.MIN_VALUE;
		else
			return (short) ((value-avg) * Short.MAX_VALUE / (reverse?-range:range));
	}
	
	/**
	 * Convert input value to byte range
	 * @param value received input
	 * @return input value as byte
	 */
	public byte getByte(int value) {
		if (Math.abs(value-avg)<flat)
			return 0;
		else if (value>avg+range)
			return reverse?Byte.MIN_VALUE:Byte.MAX_VALUE;
		else if (value<avg-range)
			return reverse?Byte.MAX_VALUE:Byte.MIN_VALUE;
		else
			return (byte) ((value-avg) * Byte.MAX_VALUE / (reverse?-range:range));
	}
	
	/**
	 * Convert input value to direction
	 * @param value received input
	 * @return input value as direction
	 */
	public int getDirection(int value) {
		if (value>(avg+range/4))
			return (reverse?DOWN:UP);
		else if (value<(avg-range/4))
			return (reverse?UP:DOWN);
		else
			return NONE;
	}
	
}
