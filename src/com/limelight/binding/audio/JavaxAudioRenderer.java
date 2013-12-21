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
	private int channelCount;
	private int sampleRate;
	
	public static final int DEFAULT_BUFFER_SIZE = 4096;
	public static final int STAGING_BUFFERS = 3; // 3 complete frames of audio
	
	@Override
	public void playDecodedAudio(short[] pcmData, int offset, int length) {
		if (soundLine != null) {
			// Queue the decoded samples into the staging sound buffer
			soundBuffer.queue(new ShortBufferDescriptor(pcmData, offset, length));
			
			// If there's space available in the sound line, pull some data out
			// of the staging buffer and write it to the sound line
			int available = soundLine.available();
			
			// Kinda jank. If the queued is larger than available, we are going to have a delay
			// so we increase the buffer size
			if (available < soundBuffer.size()) {
				System.out.println("buffer too full, buffer size: " + soundLine.getBufferSize());
				int currentBuffer = soundLine.getBufferSize();
				soundLine.close();
				createSoundLine(currentBuffer*2);
				available = soundLine.available();
				System.out.println("creating new line with buffer size: " + soundLine.getBufferSize());
			}
			
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
	
	private void createSoundLine(int bufferSize) {
		AudioFormat audioFormat = new AudioFormat(sampleRate, 16, channelCount, true, true);
		DataLine.Info info = new DataLine.Info(SourceDataLine.class, audioFormat);
		try {
			soundLine = (SourceDataLine) AudioSystem.getLine(info);
			soundLine.open(audioFormat, bufferSize);
			soundLine.start();
			lineBuffer = new byte[soundLine.getBufferSize()];
			soundBuffer = new SoundBuffer(STAGING_BUFFERS);
		} catch (LineUnavailableException e) {
			soundLine = null;
		}
	}

	@Override
	public void streamInitialized(int channelCount, int sampleRate) {
		this.channelCount = channelCount;
		this.sampleRate = sampleRate;
		createSoundLine(DEFAULT_BUFFER_SIZE);
	}

}
