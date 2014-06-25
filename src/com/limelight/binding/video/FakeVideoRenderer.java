package com.limelight.binding.video;

import com.limelight.LimeLog;
import com.limelight.nvstream.av.ByteBufferDescriptor;
import com.limelight.nvstream.av.DecodeUnit;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;

/**
 * Implementation of a video decoder and renderer.
 * @author Iwan Timmer
 */
public class FakeVideoRenderer extends AbstractVideoRenderer {

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
	public void stop() {
		super.stop();
		
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
	public void decodeUnit(DecodeUnit decodeUnit) {
		if (out!=null) {
			try {
				for (ByteBufferDescriptor buf:decodeUnit.getBufferList())
					out.write(buf.data, buf.offset, buf.length);
			} catch (IOException e) {
				LimeLog.severe(e.getMessage());
			}
		}
	}

}
