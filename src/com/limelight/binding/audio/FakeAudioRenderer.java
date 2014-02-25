package com.limelight.binding.audio;

import com.limelight.nvstream.av.audio.AudioRenderer;

/**
 * Fake implementation for audio renderer.
 * @author Iwan Timmer
 */
public class FakeAudioRenderer implements AudioRenderer {

	@Override
	public void streamInitialized(int channelCount, int sampleRate) {
	}

	@Override
	public void playDecodedAudio(byte[] audioData, int offset, int length) {
	}

	@Override
	public void streamClosing() {
	}
	
	@Override
	public int getCapabilities() {
		return CAPABILITY_DIRECT_SUBMIT;
	}
	
}