package com.limelight.binding.audio;

import com.limelight.LimeLog;
import com.limelight.nvstream.av.audio.AudioDecoderRenderer;

/**
 * Audio renderer implementation
 * @author Iwan Timmer
 */
public class AlsaAudioDecoderRenderer implements AudioDecoderRenderer {
	
	private String device;
	private byte[] decodedData;
	
	public AlsaAudioDecoderRenderer(String device) {
		this.device = device;
		this.decodedData = new byte[OpusDecoder.getMaxOutputShorts()*2];
		
		int err;
		
		err = OpusDecoder.init();
		if (err != 0) {
			throw new IllegalStateException("Opus decoder failed to initialize");
		}
	}

	@Override
	public boolean streamInitialize() {
		return AlsaAudio.init(OpusDecoder.getChannelCount(), OpusDecoder.getSampleRate(), device) == 0;
	}

	@Override
	public void playAudio(byte[] bytes, int offset, int length) {
		int decodeLen = OpusDecoder.decode(bytes, offset, length, decodedData);
		
		if (decodeLen > 0) {
			//Value of decode is frames (shorts) decoded per channel
			decodeLen *= 2*OpusDecoder.getChannelCount();
			int rc = AlsaAudio.play(decodedData, 0, decodeLen);
			
			if (rc<0)
				LimeLog.warning("Alsa error from writei: "+rc);
			else if (rc!=length/4)
				LimeLog.warning("Alsa short write, write "+rc+" frames");
		}
	}

	@Override
	public void streamClosing() {
		AlsaAudio.close();
	}
	
}
