package com.limelight.binding;

import com.limelight.binding.audio.JavaxAudioRenderer;
import com.limelight.binding.video.SwingCpuDecoderRenderer;
import com.limelight.nvstream.av.audio.AudioRenderer;
import com.limelight.nvstream.av.video.VideoDecoderRenderer;

public class PlatformBinding {
	public static VideoDecoderRenderer getVideoDecoderRenderer() {
		return new SwingCpuDecoderRenderer();
	}
	
	public static String getDeviceName() {
        return "foobar";
	}
	
	public static AudioRenderer getAudioRenderer() {
		return new JavaxAudioRenderer();
	}
}
