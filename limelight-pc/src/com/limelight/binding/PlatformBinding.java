package com.limelight.binding;

import java.net.InetAddress;
import java.net.UnknownHostException;

import com.limelight.binding.audio.JavaxAudioRenderer;
import com.limelight.binding.video.SwingCpuDecoderRenderer;
import com.limelight.nvstream.av.audio.AudioRenderer;
import com.limelight.nvstream.av.video.VideoDecoderRenderer;

public class PlatformBinding {
	public static VideoDecoderRenderer getVideoDecoderRenderer() {
		return new SwingCpuDecoderRenderer();
	}
	
	public static String getDeviceName() {
		try {
			return InetAddress.getLocalHost().getHostName();
		} catch (UnknownHostException e) {
			return "LimelightPC";
		}
	}
	
	public static AudioRenderer getAudioRenderer() {
		return new JavaxAudioRenderer();
	}
}
