package com.limelight.binding.video;

import com.limelight.nvstream.av.video.VideoDecoderRenderer;
import com.limelight.nvstream.av.video.VideoDepacketizer;

/**
 * Implementation of a video decoder and renderer.
 * @author Iwan Timmer
 */
public class FakeVideoRenderer implements VideoDecoderRenderer {
	
	private Thread thread;
	private boolean running;

	@Override
	public void setup(int width, int height, int redrawRate, Object renderTarget, int drFlags) {
	}
	
	@Override
	public void start(final VideoDepacketizer depacketizer) {
		thread = new Thread(new Runnable() {
			@Override
			public void run() {
				while (running) {
					try {
						depacketizer.takeNextDecodeUnit();
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
	}

	@Override
	public void release() {
	}

	@Override
	public int getCapabilities() {
		return 0;
	}

}
