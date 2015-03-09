package com.limelight.binding.audio;

import com.limelight.nvstream.av.audio.AudioDecoderRenderer;

/**
 * Fake implementation for audio renderer.
 * @author Iwan Timmer
 */
public class FakeAudioDecoderRenderer implements AudioDecoderRenderer {
	
	private int dataSize;
	private long last;

	@Override
	public boolean streamInitialize() {
		System.out.println("Fake audio output");
		return true;
	}

	@Override
	public void playAudio(byte[] audioData, int offset, int length) {
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
	
}