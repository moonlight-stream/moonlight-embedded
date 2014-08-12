package com.limelight.binding.video;

/**
 * JNI Decoder bindings
 * @author Iwan Timmer
 */
public class ImxDecoder {
	static {
		System.loadLibrary("nv_imx_dec");
	}
	
	public static native int init();
	
	public static native void destroy();
	
	public static native int decode(byte[] indata, int inoff, int inlen, boolean last);
}
