package com.limelight.binding.audio;

import com.limelight.nvstream.av.audio.AudioRenderer;

/**
 * Fake implementation for audio renderer.
 * @author Iwan Timmer
 */
public class FakeAudioRenderer implements AudioRenderer {
	
	private int dataSize;
	private long last;

	@Override
	public boolean streamInitialized(int channelCount, int sampleRate) {
		System.out.println("Fake " + channelCount + " channel " + sampleRate + " samplerate audio output");
		last = System.currentTimeMillis();
		return true;
	}

	@Override
	public void playDecodedAudio(byte[] audioData, int offset, int length) {
		if (System.currentTimeMillis()>last+2000) {
			int bitrate = (dataSize/2)/1024;
			System.out.println("Audio " + bitrate + "kB/s");
			dataSize = 0;
			last = System.currentTimeMillis();
		}
		dataSize += length;
	}

	@Override
	public void streamClosing() {
	}
	
	@Override
	public int getCapabilities() {
		return CAPABILITY_DIRECT_SUBMIT;
	}
	
}