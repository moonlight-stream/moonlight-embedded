package com.limelight.binding.video;

import com.limelight.LimeLog;
import com.limelight.nvstream.av.ByteBufferDescriptor;
import com.limelight.nvstream.av.DecodeUnit;
import com.limelight.nvstream.av.video.VideoDecoderRenderer;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;

/**
 * Implementation of a video decoder and renderer.
 * @author Iwan Timmer
 */
public class FakeVideoRenderer implements VideoDecoderRenderer {
	
	private int dataSize;
	private long last;
	
	private OutputStream out;

	public FakeVideoRenderer(String videoFile) {
		try {
			if (videoFile!=null)
				out = new FileOutputStream(videoFile);
		} catch (FileNotFoundException e) {
			LimeLog.severe(e.getMessage());
		}		
	}

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
		try {
			if (out!=null)
				out.close();
		} catch (IOException e) {
			LimeLog.severe(e.getMessage());
		}
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
		
		if (out!=null) {
			try {
				for (ByteBufferDescriptor buf:decodeUnit.getBufferList())
					out.write(buf.data, buf.offset, buf.length);
			} catch (IOException e) {
				LimeLog.severe(e.getMessage());
				return false;
			}
		}
		
		return true;
	}
	
	@Override
	public int getCapabilities() {
		return CAPABILITY_DIRECT_SUBMIT;
	}
	
}
