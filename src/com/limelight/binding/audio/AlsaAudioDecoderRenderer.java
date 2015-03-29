package com.limelight.binding.audio;

import com.limelight.LimeLog;
import com.limelight.nvstream.av.audio.AudioDecoderRenderer;

/**
 * Audio renderer implementation
 * @author Iwan Timmer
 */
public class AlsaAudioDecoderRenderer implements AudioDecoderRenderer {
	
	private final static int CHANNEL_COUNT = 2;
	private final static int SAMPLE_RATE = 48000;

	/* Number of 16 bits frames */
	private final static int FRAME_SIZE = 240;

	private String device;
	private byte[] decodedData;
	
	public AlsaAudioDecoderRenderer(String device) {
		this.device = device;
		this.decodedData = new byte[FRAME_SIZE * CHANNEL_COUNT * 2];
	}

	@Override
	public boolean streamInitialize() {
		return OpusDecoder.init(CHANNEL_COUNT, SAMPLE_RATE) == 0 && AlsaAudio.init(CHANNEL_COUNT, SAMPLE_RATE, device) == 0;
	}

	@Override
	public void playAudio(byte[] bytes, int offset, int length) {
		int decodeLen = OpusDecoder.decode(bytes, offset, length, FRAME_SIZE, decodedData);
		
		if (decodeLen > 0) {
			//Value of decode is frames (shorts) decoded per channel
			decodeLen *= CHANNEL_COUNT * 2;
			int rc = AlsaAudio.play(decodedData, 0, decodeLen);
			
			if (rc<0)
				LimeLog.warning("Alsa error from writei: "+rc);
			else if (rc!=decodeLen/4)
				LimeLog.warning("Alsa short write, write "+rc+" frames");
		} else {
			LimeLog.warning("Opus error from decode: "+decodeLen);
		}
	}

	@Override
	public void streamClosing() {
		AlsaAudio.close();
	}
	
}
