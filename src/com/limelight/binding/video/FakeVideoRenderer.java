package com.limelight.binding.video;

import com.limelight.nvstream.av.DecodeUnit;
import com.limelight.nvstream.av.video.VideoDecoderRenderer;

/**
 * Implementation of a video decoder and renderer.
 * @author Iwan Timmer
 */
public class FakeVideoRenderer implements VideoDecoderRenderer {

	@Override
	public void setup(int width, int height, int redrawRate, Object renderTarget, int drFlags) {
	}

	@Override
	public void start() {
	}

	@Override
	public void stop() {
	}

	@Override
	public void release() {
	}

	@Override
	public boolean submitDecodeUnit(DecodeUnit decodeUnit) {
		return true;
	}
	
}
