package com.limelight.binding.audio;

public class OpusDecoder {
	static {
		System.loadLibrary("nv_opus_dec");
	}
	
	public static native int init(int channelCoumt, int sampleRate);
	public static native void destroy();
	public static native int decode(byte[] indata, int inoff, int inlen, int frameSize, byte[] outpcmdata);
}
