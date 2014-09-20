package com.limelight.binding.video;

import com.limelight.LimeLog;
import com.limelight.nvstream.av.ByteBufferDescriptor;
import com.limelight.nvstream.av.DecodeUnit;

import java.util.List;

/**
 * Implementation of a video decoder and renderer.
 * @author Iwan Timmer
 */
public class ImxDecoderRenderer extends AbstractVideoRenderer {
	
	@Override
	public boolean setup(int width, int height, int redrawRate, Object renderTarget, int drFlags) {
		return ImxDecoder.init() == 0;
	}

	@Override
	public void release() {
		ImxDecoder.destroy();
	}

	@Override
	public void decodeUnit(DecodeUnit decodeUnit) {
		List<ByteBufferDescriptor> units = decodeUnit.getBufferList();
		
		boolean ok = true;
		for (int i=0;i<units.size();i++) {
			ByteBufferDescriptor bbd = units.get(i);
			if (ok) {
				int ret = ImxDecoder.decode(bbd.data, bbd.offset, bbd.length, i == (units.size()-1));
				if (ret != 0) {
					LimeLog.severe("Error code during decode: " + ret);
					ok = false;
				}
			}
		}
	}
}
