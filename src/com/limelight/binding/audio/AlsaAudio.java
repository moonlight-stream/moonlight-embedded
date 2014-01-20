package com.limelight.binding.audio;

/**
 * JNI Alsa bindings
 * @author Iwan Timmer
 */
public class AlsaAudio {
	static {
		System.loadLibrary("nv_alsa");
	}
	
	public static native int init(int channelCount, int sampleRate);
	
	public static native void close();
	
	public static native int play(byte[] indata, int inoff, int inlen);
}
