package com.limelight.binding.audio;

import com.limelight.nvstream.av.audio.AudioRenderer;

/**
 * Audio renderer implementation
 * @author Iwan Timmer
 */
public class AlsaAudioRenderer implements AudioRenderer {

	@Override
	public void streamInitialized(int channelCount, int sampleRate) {
		int ret = AlsaAudio.init(channelCount, sampleRate);
		if (ret != 0)
			throw new IllegalStateException("AVC decoder initialization failure: "+ret);
	}

	@Override
	public void playDecodedAudio(byte[] bytes, int offset, int length) {
		AlsaAudio.play(bytes, offset, length);
	}

	@Override
	public void streamClosing() {
		AlsaAudio.close();
	}
	
}
