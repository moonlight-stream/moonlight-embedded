package com.limelight.binding.video;

/**
 * JNI Decoder bindings
 * @author Iwan Timmer
 */
public class OmxDecoder {
	
	public static boolean load() {
		try {
			System.loadLibrary("nv_omx_dec");
		} catch (Throwable e) {
			return false;
		}
		return true;
	}
	
	public static native int init();
	
	public static native void stop();
	
	public static native void destroy();
	
	public static native int decode(byte[] indata, int inoff, int inlen, boolean last);
}
