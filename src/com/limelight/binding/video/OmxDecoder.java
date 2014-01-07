package com.limelight.binding.video;

import com.limelight.binding.LibraryHelper;

public class OmxDecoder {
	static {
		LibraryHelper.loadNativeLibrary("nv_omx_dec");
	}
	
	public static native int init();
	
	public static native void stop();
	
	public static native void destroy();
	
	public static native int decode(byte[] indata, int inoff, int inlen, boolean last);
}
