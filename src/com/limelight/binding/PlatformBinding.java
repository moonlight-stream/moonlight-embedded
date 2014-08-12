package com.limelight.binding;

import java.net.InetAddress;
import java.net.UnknownHostException;

import com.limelight.binding.audio.AlsaAudioRenderer;
import com.limelight.binding.video.ImxDecoderRenderer;
import com.limelight.binding.crypto.PcCryptoProvider;
import com.limelight.nvstream.av.audio.AudioRenderer;
import com.limelight.nvstream.av.video.VideoDecoderRenderer;
import com.limelight.nvstream.http.LimelightCryptoProvider;

/**
 * Used for platform-specific video/audio bindings.
 * @author Cameron Gutman<br>
 * Iwan Timmer
 */
public class PlatformBinding {
	/**
	 * Gets an instance of a video decoder/renderer.
	 * @return a video decoder and renderer
	 */
	public static VideoDecoderRenderer getVideoDecoderRenderer() {
		return new ImxDecoderRenderer();
	}
	
	/**
	 * Gets the name of this device. 
	 * <br>Currently, the hostname of the system.
	 * @return the name of this device
	 */
	public static String getDeviceName() {
		try {
			return InetAddress.getLocalHost().getHostName();
		} catch (UnknownHostException e) {
			return "LimelightPi";
		}
	}
	
	/**
	 * Gets an instance of an audio decoder/renderer.
	 * @return an audio decoder and renderer
	 */
	public static AudioRenderer getAudioRenderer(String device) {
		return new AlsaAudioRenderer(device);
	}
	
	/**
	 * Gets an instance of a crypto provider
	 * @return a PcCryptoProvider object
	 */
	public static LimelightCryptoProvider getCryptoProvider() {
		return new PcCryptoProvider();
	}
}
