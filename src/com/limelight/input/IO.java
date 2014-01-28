package com.limelight.input;

/**
 * IO bindings
 * @author Iwan Timmer
 */
public class IO {
	static {
		System.loadLibrary("nv_io");
	}
	
	public static native boolean ioctl(String filename, byte[] buffer, int request);
	
}
