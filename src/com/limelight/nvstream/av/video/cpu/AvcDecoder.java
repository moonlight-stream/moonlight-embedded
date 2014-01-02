package com.limelight.nvstream.av.video.cpu;

import com.limelight.binding.LibraryHelper;

public class AvcDecoder {
	static {
		// Windows uses runtime linking for ffmpeg libraries
		if (System.getProperty("os.name").contains("Windows")) {
			LibraryHelper.loadNativeLibrary("avutil-52");
			LibraryHelper.loadNativeLibrary("postproc-52");
			LibraryHelper.loadNativeLibrary("pthreadVC2");
			LibraryHelper.loadNativeLibrary("swresample-0");
			LibraryHelper.loadNativeLibrary("swscale-2");
			LibraryHelper.loadNativeLibrary("avcodec-55");
			LibraryHelper.loadNativeLibrary("avformat-55");
			LibraryHelper.loadNativeLibrary("avfilter-3");
		}
		
		LibraryHelper.loadNativeLibrary("nv_avc_dec");
	}
	
	/** Disables the deblocking filter at the cost of image quality */
	public static final int DISABLE_LOOP_FILTER = 0x1;
	/** Uses the low latency decode flag (disables multithreading) */
	public static final int LOW_LATENCY_DECODE = 0x2;
	/** Threads process each slice, rather than each frame */
	public static final int SLICE_THREADING = 0x4;
	/** Uses nonstandard speedup tricks */
	public static final int FAST_DECODE = 0x8;
	/** Uses bilinear filtering instead of bicubic */
	public static final int BILINEAR_FILTERING = 0x10;
	/** Uses a faster bilinear filtering with lower image quality */
	public static final int FAST_BILINEAR_FILTERING = 0x20;
	/** Disables color conversion (output is NV21) */
	public static final int NO_COLOR_CONVERSION = 0x40;
	
	public static native int init(int width, int height, int perflvl, int threadcount);
	public static native void destroy();
	
	// Rendering API when NO_COLOR_CONVERSION == 0
	public static native boolean setRenderTarget(Object androidSurface);
	public static native boolean getRgbFrameInt(int[] rgbFrame, int bufferSize);
	public static native boolean getRgbFrame(byte[] rgbFrame, int bufferSize);
	public static native boolean redraw();

	// Rendering API when NO_COLOR_CONVERSION == 1
	public static native boolean getRawFrame(byte[] yuvFrame, int bufferSize);
	
	public static native int getInputPaddingSize();
	public static native int decode(byte[] indata, int inoff, int inlen);
}
