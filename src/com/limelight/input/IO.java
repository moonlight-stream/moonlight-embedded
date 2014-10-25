package com.limelight.input;

/**
 * IO bindings
 * @author Iwan Timmer
 */
public class IO {
	
	public final static int READ_ONLY = 2;
	
	static {
		System.loadLibrary("nv_io");
	}
	
	public static native boolean ioctl(String filename, byte[] buffer, int request);
	
	public static int getRequest(int dir, int type, int nr, int size) {
		return (dir << 30) | (size << 16) | (type << 8) | nr;
	}
	
}
