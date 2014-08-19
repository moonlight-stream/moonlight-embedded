package com.limelight.binding.video;

/**
 * JNI Decoder bindings
 * @author Iwan Timmer
 */
public class ImxDecoder {
	
	public static boolean load() {
		try {
			System.loadLibrary("nv_imx_dec");
		} catch (Throwable e) {
			return false;
		}
		return true;
	}
	
	public static native int init();
	
	public static native void destroy();
	
	public static native int decode(byte[] indata, int inoff, int inlen, boolean last);
}
