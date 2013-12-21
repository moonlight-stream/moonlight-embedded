package com.limelight.binding.audio;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.DataLine;
import javax.sound.sampled.LineUnavailableException;
import javax.sound.sampled.SourceDataLine;

import com.limelight.nvstream.av.ShortBufferDescriptor;
import com.limelight.nvstream.av.audio.AudioRenderer;

public class JavaxAudioRenderer implements AudioRenderer {

	private SourceDataLine soundLine;
	private SoundBuffer soundBuffer;
	private byte[] lineBuffer;
	
	public static final int STAGING_BUFFERS = 3; // 3 complete frames of audio
	
	@Override
	public void playDecodedAudio(short[] pcmData, int offset, int length) {
		if (soundLine != null) {
			// Queue the decoded samples into the staging sound buffer
			soundBuffer.queue(new ShortBufferDescriptor(pcmData, offset, length));
			
			// If there's space available in the sound line, pull some data out
			// of the staging buffer and write it to the sound line
			int available = soundLine.available();
			if (available > 0) {
				int written = soundBuffer.fill(lineBuffer, 0, available);
				if (written > 0) {
					soundLine.write(lineBuffer, 0, written);
				}
			}
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
		DataLine.Info info = new DataLine.Info(SourceDataLine.class, audioFormat);
		try {
			soundLine = (SourceDataLine) AudioSystem.getLine(info);
			
			// Java's OS X mixer performs very badly with the default buffer size
			if (System.getProperty("os.name").contains("Mac OS X")) {
				soundLine.open(audioFormat, 16384);
			}
			else {
				soundLine.open(audioFormat);
			}
			
			soundLine.start();
			
			lineBuffer = new byte[soundLine.getBufferSize()];
			soundBuffer = new SoundBuffer(STAGING_BUFFERS);
		} catch (LineUnavailableException e) {
			soundLine = null;
		}
	}

}
