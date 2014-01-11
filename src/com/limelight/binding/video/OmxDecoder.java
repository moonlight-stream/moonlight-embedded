package com.limelight.binding.video;

/**
 * JNI Decoder bindings
 * @author Iwan Timmer
 */
public class OmxDecoder {
	static {
		System.loadLibrary("nv_omx_dec");
	}
	
	public static native int init();
	
	public static native void stop();
	
	public static native void destroy();
	
	public static native int decode(byte[] indata, int inoff, int inlen, boolean last);
}
