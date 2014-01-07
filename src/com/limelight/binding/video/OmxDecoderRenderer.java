package com.limelight.binding.video;

import com.limelight.nvstream.av.ByteBufferDescriptor;
import com.limelight.nvstream.av.DecodeUnit;
import com.limelight.nvstream.av.video.VideoDecoderRenderer;

import java.nio.ByteBuffer;
import java.util.List;

/**
 * Implementation of a video decoder and renderer.
 * @author Iwan Timmer
 */
public class OmxDecoderRenderer implements VideoDecoderRenderer {

	@Override
	public void setup(int width, int height, int redrawRate, Object renderTarget, int drFlags) {
		int err = OmxDecoder.init();
		if (err != 0) {
			throw new IllegalStateException("AVC decoder initialization failure: "+err);
		}
		
		System.out.println("Using omx decoding");
	}

	@Override
	public void start() {
		System.out.println("Start omx rendering");
	}

	@Override
	public void stop() {
		OmxDecoder.stop();
	}

	@Override
	public void release() {
		OmxDecoder.destroy();
	}

	@Override
	public boolean submitDecodeUnit(DecodeUnit decodeUnit) {
		boolean ok = true;
		List<ByteBufferDescriptor> units = decodeUnit.getBufferList();
		for (int i=0;i<units.size();i++) {
			ByteBufferDescriptor bbd = units.get(i);
			if (ok)
				ok = (OmxDecoder.decode(bbd.data, bbd.offset, bbd.length, i == (units.size()-1)) == 0);
		}
		
		return ok;
	}
	
}
