package com.limelight.binding.audio;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.DataLine;
import javax.sound.sampled.LineUnavailableException;
import javax.sound.sampled.SourceDataLine;

import com.limelight.nvstream.av.ByteBufferDescriptor;
import com.limelight.nvstream.av.audio.AudioRenderer;
import java.nio.ByteOrder;
import java.util.LinkedList;

/**
 * Audio renderer implementation
 * @author Cameron Gutman<br>
 * Iwan Timmer
 */
public class JavaxAudioRenderer implements AudioRenderer {

	private SourceDataLine soundLine;
	private LinkedList<ByteBufferDescriptor> soundBuffer;
	private byte[] lineBuffer;
	private int channelCount;
	private int sampleRate;
	private boolean reallocateLines;
	
	public static final int DEFAULT_BUFFER_SIZE = 0;
	public static final int STARING_BUFFER_SIZE = 4096;
	public static final int STAGING_BUFFERS = 3; // 3 complete frames of audio
	
	/**
	 * Takes some audio data and writes it out to the renderer.
	 * @param pcmData the array that contains the audio data
	 * @param offset the offset at which the data starts in the array
	 * @param length the length of data to be rendered
	 */
	@Override
	public void playDecodedAudio(byte[] pcmData, int offset, int length) {
		if (soundLine != null) {
			// Queue the decoded samples into the staging sound buffer
			if (soundBuffer.size() > STAGING_BUFFERS) {
				soundBuffer.removeFirst();
			}
		
			soundBuffer.addLast(new ByteBufferDescriptor(pcmData, offset, length));
			
			int available = soundLine.available();
			if (reallocateLines) {
				// Kinda jank. If the queued is larger than available, we are going to have a delay
				// so we increase the buffer size
				int size = 0;
				for (ByteBufferDescriptor desc : soundBuffer) {
					size += desc.length;
				}
				
				if (available < size) {
					System.out.println("buffer too full, buffer size: " + soundLine.getBufferSize());
					int currentBuffer = soundLine.getBufferSize();
					soundLine.close();
					createSoundLine(currentBuffer*2);
					if (soundLine != null) {
						available = soundLine.available();
						System.out.println("creating new line with buffer size: " + soundLine.getBufferSize());
					}
					else {
						available = 0;
						System.out.println("failed to create sound line");
					}
				}
			}
			
			// If there's space available in the sound line, pull some data out
			// of the staging buffer and write it to the sound line
			
			while (available > 0 && !soundBuffer.isEmpty()) {
				ByteBufferDescriptor buff = soundBuffer.peek();
				if (buff.length > available) {
					break;
				}
				
				available -= soundLine.write(buff.data, buff.offset, buff.length);
				
				soundBuffer.remove();
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
			lineBuffer = new byte[soundLine.getBufferSize()];
			soundBuffer = new LinkedList<ByteBufferDescriptor>();
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
		
		createSoundLine(STARING_BUFFER_SIZE);
		reallocateLines = true;
	}

}
