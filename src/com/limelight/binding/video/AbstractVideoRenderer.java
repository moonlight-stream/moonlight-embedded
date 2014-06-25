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
	
	@Override
	public void start(final VideoDepacketizer depacketizer) {
		last = System.currentTimeMillis();
		thread = new Thread(new Runnable() {
			@Override
			public void run() {
				while (running) {
					try {
						DecodeUnit decodeUnit = depacketizer.takeNextDecodeUnit();
						
						dataSize += decodeUnit.getDataLength();
						decodeUnit(decodeUnit);
						
						if (System.currentTimeMillis()>last+2000) {
							int bitrate = (dataSize/2)/1024;
							long latency = System.currentTimeMillis()-decodeUnit.getReceiveTimestamp();
							System.out.println("Video " + bitrate + "kB/s " + latency + "ms");
							dataSize = 0;
							last = System.currentTimeMillis();
						}
					} catch (InterruptedException ex) {	}
				}
			}
		});
		running = true;
		thread.start();
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

}
