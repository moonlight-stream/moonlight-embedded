package com.limelight.binding.video;

import com.limelight.nvstream.av.DecodeUnit;
import com.limelight.nvstream.av.video.VideoDecoderRenderer;

/**
 * Implementation of a video decoder and renderer.
 * @author Iwan Timmer
 */
public class FakeVideoRenderer implements VideoDecoderRenderer {
	
	private int dataSize;
	private long last;

	@Override
	public void setup(int width, int height, int redrawRate, Object renderTarget, int drFlags) {
		System.out.println("Fake " + width + "x" + height + " " + redrawRate + "fps video output");
	}

	@Override
	public void start() {
		last = System.currentTimeMillis();
	}

	@Override
	public void stop() {
	}

	@Override
	public void release() {
	}

	@Override
	public boolean submitDecodeUnit(DecodeUnit decodeUnit) {
		if (System.currentTimeMillis()>last+2000) {
			int bitrate = (dataSize/2)/1024;
			System.out.println("Video " + bitrate + "kB/s");
			dataSize = 0;
			last = System.currentTimeMillis();
		}
		dataSize += decodeUnit.getDataLength();
		
		return true;
	}
	
	@Override
	public int getCapabilities() {
		return CAPABILITY_DIRECT_SUBMIT;
	}
	
}
