package com.limelight.binding.audio;

import java.nio.ByteBuffer;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.DataLine;
import javax.sound.sampled.LineUnavailableException;
import javax.sound.sampled.SourceDataLine;

import com.limelight.nvstream.av.audio.AudioRenderer;

public class JavaxAudioRenderer implements AudioRenderer {

	private SourceDataLine soundLine;
	
	@Override
	public void playDecodedAudio(short[] pcmData, int offset, int length) {
		if (soundLine != null) {
			byte[] pcmDataBytes = new byte[length * 2];
			ByteBuffer.wrap(pcmDataBytes).asShortBuffer().put(pcmData, offset, length);
			soundLine.write(pcmDataBytes, 0, pcmDataBytes.length);	
		}
	}

	@Override
	public void streamClosing() {
		if (soundLine != null) {
			soundLine.close();
		}
	}

	@Override
	public void streamInitialized(int channelCount, int sampleRate) {
		AudioFormat audioFormat = new AudioFormat(sampleRate, 16, channelCount, true, true);
		DataLine.Info info = new DataLine.Info(SourceDataLine.class, audioFormat, 1);
		try {
			soundLine = (SourceDataLine) AudioSystem.getLine(info);
			soundLine.open(audioFormat);
			soundLine.start();
		} catch (LineUnavailableException e) {
			soundLine = null;
		}
	}

}
