package com.limelight.binding.audio;

import java.nio.ByteBuffer;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.DataLine;
import javax.sound.sampled.LineUnavailableException;
import javax.sound.sampled.SourceDataLine;

import com.limelight.nvstream.av.audio.AudioRenderer;
import com.limelight.nvstream.av.audio.OpusDecoder;

/**
 * Audio renderer implementation
 * @author Cameron Gutman
 */
public class JavaxAudioRenderer implements AudioRenderer {

	private SourceDataLine soundLine;
	
	/**
	 * Takes some audio data and writes it out to the renderer.
	 * @param pcmData the array that contains the audio data
	 * @param offset the offset at which the data starts in the array
	 * @param length the length of data to be rendered
	 */
	@Override
	public void playDecodedAudio(short[] pcmData, int offset, int length) {
		if (soundLine != null) {
			byte[] pcmDataBytes = new byte[length * 2];
			ByteBuffer.wrap(pcmDataBytes).asShortBuffer().put(pcmData, offset, length);
			if (soundLine.available() < length) {
				soundLine.write(pcmDataBytes, 0, soundLine.available());
			}
			else {
				soundLine.write(pcmDataBytes, 0, pcmDataBytes.length);
			}
		}
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

	/**
	 * The callback for the audio stream being initialized and starting to receive.
	 * @param channelCount the number of channels in the audio
	 * @param sampleRate the sample rate for the audio.
	 */
	@Override
	public void streamInitialized(int channelCount, int sampleRate) {
		AudioFormat audioFormat = new AudioFormat(sampleRate, 16, channelCount, true, true);
		DataLine.Info info = new DataLine.Info(SourceDataLine.class, audioFormat, OpusDecoder.getMaxOutputShorts());
		try {
			soundLine = (SourceDataLine) AudioSystem.getLine(info);
			soundLine.open(audioFormat, OpusDecoder.getMaxOutputShorts()*4*2);
			soundLine.start();
		} catch (LineUnavailableException e) {
			soundLine = null;
		}
	}

}
