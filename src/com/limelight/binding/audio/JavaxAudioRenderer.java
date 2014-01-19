package com.limelight.binding.audio;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.DataLine;
import javax.sound.sampled.LineUnavailableException;
import javax.sound.sampled.SourceDataLine;

import com.limelight.nvstream.av.audio.AudioRenderer;
import java.nio.ByteOrder;

/**
 * Audio renderer implementation
 * @author Cameron Gutman<br>
 * Iwan Timmer
 */
public class JavaxAudioRenderer implements AudioRenderer {

	private SourceDataLine soundLine;
	private int channelCount;
	private int sampleRate;
	
	public static final int DEFAULT_BUFFER_SIZE = 4096;
	
	/**
	 * Takes some audio data and writes it out to the renderer.
	 * @param pcmData the array that contains the audio data
	 * @param offset the offset at which the data starts in the array
	 * @param length the length of data to be rendered
	 */
	@Override
	public void playDecodedAudio(byte[] pcmData, int offset, int length) {
		soundLine.write(pcmData, offset, length);
	}

	/**
	 * Callback for when the stream session is closing and the audio renderer should stop.
	 */
	@Override
	public void streamClosing() {
		if (soundLine != null) {
			soundLine.close();
		}
	}

	private void createSoundLine(int bufferSize) {
		AudioFormat audioFormat = new AudioFormat(sampleRate, 16, channelCount, true, ByteOrder.nativeOrder()==ByteOrder.BIG_ENDIAN);
		
		DataLine.Info info;
		
		if (bufferSize == DEFAULT_BUFFER_SIZE) {
			info = new DataLine.Info(SourceDataLine.class, audioFormat);
		}
		else {
			info = new DataLine.Info(SourceDataLine.class, audioFormat, bufferSize);
		}
		
		try {
			soundLine = (SourceDataLine) AudioSystem.getLine(info);
			
			if (bufferSize == DEFAULT_BUFFER_SIZE) {
				soundLine.open(audioFormat);
			}
			else {
				soundLine.open(audioFormat, bufferSize);
			}
			
			soundLine.start();
		} catch (LineUnavailableException e) {
			soundLine = null;
		}
	}

	/**
	 * The callback for the audio stream being initialized and starting to receive.
	 * @param channelCount the number of channels in the audio
	 * @param sampleRate the sample rate for the audio.
	 */
	@Override
	public void streamInitialized(int channelCount, int sampleRate) {
		this.channelCount = channelCount;
		this.sampleRate = sampleRate;
		
		createSoundLine(DEFAULT_BUFFER_SIZE);
	}

}
