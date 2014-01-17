package com.limelight.binding.video;

import com.limelight.nvstream.av.ByteBufferDescriptor;
import com.limelight.nvstream.av.DecodeUnit;
import com.limelight.nvstream.av.video.VideoDecoderRenderer;

import java.util.List;

/**
 * Implementation of a video decoder and renderer.
 * @author Iwan Timmer
 */
public class OmxDecoderRenderer implements VideoDecoderRenderer {
	
	private final static int BITSTREAM_RESTRICTIONS = (int) 0xF1832C00l;

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
		
		ByteBufferDescriptor header = units.get(0);
 		if (header.data[header.offset+4] == 0x67) {
			byte last = header.data[header.length+header.offset-1];
			//Start bit before stop bit
			int shift = (Integer.numberOfLeadingZeros(last & - last) - 1)%8;
			int bitstream_restriction = BITSTREAM_RESTRICTIONS >>> shift;
			boolean twoBytes = shift==7;
			int index = header.length - (twoBytes?2:1);

			header.length += twoBytes?1:2;
			if (Integer.numberOfTrailingZeros(bitstream_restriction) < 8)
				header.length++;
			
			byte[] newArray = new byte[header.length];
			System.arraycopy(header.data, header.offset, newArray, 0, header.length-1);
			
			newArray[index] = (byte) (((int) header.data[index + header.offset] & 0xFF) | (bitstream_restriction >>> 24));
			newArray[index+1] = (byte) ((twoBytes?((int) header.data[index+1 + header.offset] & 0xFF):0) | (bitstream_restriction >>> 16));
			newArray[index+2] = (byte) (bitstream_restriction >>> 8);
			
			if (Integer.numberOfTrailingZeros(bitstream_restriction) < 8)
				newArray[index+3] = (byte) (bitstream_restriction);
			
			header.data = newArray;
			header.offset = 0;
		}
		
		for (int i=0;i<units.size();i++) {
			ByteBufferDescriptor bbd = units.get(i);
			if (ok)
				ok = (OmxDecoder.decode(bbd.data, bbd.offset, bbd.length, i == (units.size()-1)) == 0);
		}
		
		return ok;
	}
	
}
