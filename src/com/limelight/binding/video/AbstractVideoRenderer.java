package com.limelight.binding.video;

import com.limelight.LimeLog;
import com.limelight.nvstream.av.DecodeUnit;
import com.limelight.nvstream.av.video.VideoDecoderRenderer;
import com.limelight.nvstream.av.video.VideoDepacketizer;

/**
 * Abstract implementation of a video decoder.
 * @author Iwan Timmer
 */
public abstract class AbstractVideoRenderer implements VideoDecoderRenderer {
	
	private Thread thread;
	private boolean running;

	private int dataSize;
	private long last;
	
	private long endToEndLatency;
	private long decodeLatency;
	private long packets;
	private long maxLatency;
	
	@Override
	public boolean start(final VideoDepacketizer depacketizer) {
		last = System.currentTimeMillis();
		thread = new Thread(new Runnable() {
			@Override
			public void run() {
				while (running) {
					try {
						DecodeUnit decodeUnit = depacketizer.takeNextDecodeUnit();
						long latency = System.currentTimeMillis()-decodeUnit.getReceiveTimestamp();
						endToEndLatency += latency;
						
						dataSize += decodeUnit.getDataLength();
						decodeUnit(decodeUnit);

						latency = System.currentTimeMillis()-decodeUnit.getReceiveTimestamp();
						decodeLatency += latency;
						
						if (latency>maxLatency)
							maxLatency = latency;
						
						if (System.currentTimeMillis()>last+2000) {
							int bitrate = (dataSize/2)/1024;
							System.out.println("Video " + bitrate + "kB/s " + maxLatency + "ms");
							maxLatency = 0;
							dataSize = 0;
							last = System.currentTimeMillis();
						}
						depacketizer.freeDecodeUnit(decodeUnit);
					} catch (InterruptedException ex) {	}
				}
			}
		});
		running = true;
		thread.start();
		return true;
	}

	@Override
	public void stop() {
		running = false;
		thread.interrupt();
		try {
			thread.join();
		} catch (InterruptedException ex) {
			LimeLog.severe(ex.getMessage());
		}
	}

	public abstract void decodeUnit(DecodeUnit decodeUnit);
	
	@Override
	public int getCapabilities() {
		return 0;
	}
	
	@Override
	public int getAverageEndToEndLatency() {
		return (int) (endToEndLatency / packets);
	}

	@Override
	public int getAverageDecoderLatency() {
		return (int) (decodeLatency / packets);
	}

}
