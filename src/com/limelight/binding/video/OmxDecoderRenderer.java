package com.limelight.binding.video;

import com.limelight.LimeLog;
import com.limelight.nvstream.av.ByteBufferDescriptor;
import com.limelight.nvstream.av.DecodeUnit;
import com.limelight.nvstream.av.video.VideoDecoderRenderer;
import com.limelight.nvstream.av.video.VideoDepacketizer;

import org.jcodec.codecs.h264.io.model.SeqParameterSet;
import org.jcodec.codecs.h264.io.model.VUIParameters;

import java.util.List;
import java.nio.ByteBuffer;

/**
 * Implementation of a video decoder and renderer.
 * @author Iwan Timmer
 */
public class OmxDecoderRenderer extends VideoDecoderRenderer {
	
	@Override
	public boolean setup(int width, int height, int redrawRate, Object renderTarget, int drFlags) {
		return OmxDecoder.init() == 0;
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
	public void directSubmitDecodeUnit(DecodeUnit decodeUnit) {
		List<ByteBufferDescriptor> units = decodeUnit.getBufferList();
		
		ByteBufferDescriptor header = units.get(0);
		if (header.data[header.offset+4] == 0x67) {
			ByteBuffer origSpsBuf = ByteBuffer.wrap(header.data);

			// Skip to the start of the NALU data
			origSpsBuf.position(header.offset+5);

			SeqParameterSet sps = SeqParameterSet.read(origSpsBuf);

			// Set number of reference frames back to 1 as it's the minimum for bitstream restrictions
			sps.num_ref_frames = 1;

			// Set bitstream restrictions to only buffer single frame
			sps.vuiParams.bitstreamRestriction = new VUIParameters.BitstreamRestriction();
			sps.vuiParams.bitstreamRestriction.motion_vectors_over_pic_boundaries_flag = true;
			sps.vuiParams.bitstreamRestriction.max_bytes_per_pic_denom = 2;
			sps.vuiParams.bitstreamRestriction.max_bits_per_mb_denom = 1;
			sps.vuiParams.bitstreamRestriction.log2_max_mv_length_horizontal = 16;
			sps.vuiParams.bitstreamRestriction.log2_max_mv_length_vertical = 16;
			sps.vuiParams.bitstreamRestriction.num_reorder_frames = 0;
			sps.vuiParams.bitstreamRestriction.max_dec_frame_buffering = 1;

			ByteBuffer newSpsBuf = ByteBuffer.allocate(128);

			// Write the annex B header
			newSpsBuf.put(header.data, header.offset, 5);

			// Write the modified SPS to the input buffer
			sps.write(newSpsBuf);

			int ret = OmxDecoder.decode(newSpsBuf.array(), 0, newSpsBuf.position(), true);
			if (ret != 0) {
				LimeLog.severe("Error code during decode: " + ret);
			}
		}
		else {
			boolean ok = true;
			for (int i=0;i<units.size();i++) {
				ByteBufferDescriptor bbd = units.get(i);
				if (ok) {
					int ret = OmxDecoder.decode(bbd.data, bbd.offset, bbd.length, i == (units.size()-1));
					if (ret != 0) {
						LimeLog.severe("Error code during decode: " + ret);
						ok = false;
					}
				}
			}
		}
	}
	
	@Override
	public boolean start(VideoDepacketizer depacketizer) {
		throw new UnsupportedOperationException("CAPABILITY_DIRECT_SUBMIT requires directSubmitDecodeUnit()");
	}

	@Override
	public int getCapabilities() {
		return CAPABILITY_DIRECT_SUBMIT;
	}
}
