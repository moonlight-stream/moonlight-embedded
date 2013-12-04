package com.limelight.nvstream;

import java.io.IOException;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.Enumeration;
import java.util.concurrent.ThreadPoolExecutor;

import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.SwingUtilities;
import javax.xml.stream.XMLStreamException;

import com.limelight.nvstream.input.NvController;

public class NvConnection {
	private String host;
	private JFrame parent;
	private NvConnectionListener listener;
	private int drFlags;
	
	private InetAddress hostAddr;
	private NvControl controlStream;
	private NvController inputStream;
	private NvVideoStream videoStream;
	private NvAudioStream audioStream;
	
	private ThreadPoolExecutor threadPool;
	
	public NvConnection(String host, JFrame parent, NvConnectionListener listener) {
		this.host = host;
		this.parent = parent;
		this.listener = listener;
	}
	
	public static String getMacAddressString() throws SocketException {
		Enumeration<NetworkInterface> ifaceList;
		NetworkInterface selectedIface = null;

		// First look for a WLAN interface (since those generally aren't removable)
		ifaceList = NetworkInterface.getNetworkInterfaces();
		while (selectedIface == null && ifaceList.hasMoreElements()) {
			NetworkInterface iface = ifaceList.nextElement();

			if (iface.getName().startsWith("wlan") && 
				iface.getHardwareAddress() != null) {
				selectedIface = iface;
			}
		}

		// If we didn't find that, look for an Ethernet interface
		ifaceList = NetworkInterface.getNetworkInterfaces();
		while (selectedIface == null && ifaceList.hasMoreElements()) {
			NetworkInterface iface = ifaceList.nextElement();

			if (iface.getName().startsWith("eth") &&
				iface.getHardwareAddress() != null) {
				selectedIface = iface;
			}
		}
		
		// Now just find something with a MAC address
		ifaceList = NetworkInterface.getNetworkInterfaces();
		while (selectedIface == null && ifaceList.hasMoreElements()) {
			NetworkInterface iface = ifaceList.nextElement();

			if (iface.getHardwareAddress() != null) {
			selectedIface = ifaceList.nextElement();
				break;
			}
		}
		
		if (selectedIface == null) {
			return null;
		}

		byte[] macAddress = selectedIface.getHardwareAddress();
		if (macAddress != null) {
			StringBuilder addrStr = new StringBuilder();
			for (int i = 0; i < macAddress.length; i++) {
				addrStr.append(String.format("%02x", macAddress[i]));
				if (i != macAddress.length - 1) {
					addrStr.append(':');
				}
			}
			return addrStr.toString();
		}
		
		return null;
	}
	
	public void start() {
		new Thread(new Runnable() {
			@Override
			public void run() {
				try {
					hostAddr = InetAddress.getByName(host);
				} catch (UnknownHostException e) {
					e.printStackTrace();
					displayMessage(e.getMessage());
					listener.connectionTerminated(e);
					return;
				}
				
				establishConnection();
				
			}
		}).start();
	}
	
	
	private void establishConnection() {
		for (NvConnectionListener.Stage currentStage : NvConnectionListener.Stage.values())
		{
			boolean success = false;

			listener.stageStarting(currentStage);
			try {
				switch (currentStage)
				{
				case LAUNCH_APP:
					success = startSteamBigPicture();
					break;

				case HANDSHAKE:
					success = NvHandshake.performHandshake(hostAddr);
					break;
					
				case CONTROL_START:
					success = startControlStream();
					break;
					
				case VIDEO_START:
					success = startVideoStream();
					break;
					
				case AUDIO_START:
					success = startAudioStream();
					break;
					
				case CONTROL_START2:
					controlStream.startJitterPackets();
					success = true;
					break;
					
				case INPUT_START:
					success = startInputConnection();
					break;
				}
			} catch (Exception e) {
				e.printStackTrace();
				success = false;
			}
			
			if (success) {
				listener.stageComplete(currentStage);
			}
			else {
				listener.stageFailed(currentStage);
				return;
			}
		}
		
		listener.connectionStarted();
	}
	
	private boolean startSteamBigPicture() throws XMLStreamException, IOException
	{
		System.out.println(hostAddr.toString() + "\t" + getMacAddressString());
		NvHTTP h = new NvHTTP(hostAddr.toString(), "");//getMacAddressString());
		
		if (!h.getPairState()) {
			displayMessage("Device not paired with computer");
			return false;
		}
		
		int sessionId = h.getSessionId();
		int appId = h.getSteamAppId(sessionId);
		
		h.launchApp(sessionId, appId);
		
		return true;
	}
	
	private boolean startControlStream() throws IOException
	{
		controlStream = new NvControl(hostAddr, listener);
		controlStream.initialize();
		controlStream.start();
		return true;
	}
	
	private boolean startVideoStream() throws IOException
	{
		videoStream = new NvVideoStream(hostAddr, listener, controlStream);
		//videoStream.startVideoStream(video, drFlags);
		return true;
	}
	
	private boolean startAudioStream() throws IOException
	{
		audioStream = new NvAudioStream(hostAddr, listener);
		audioStream.startAudioStream();
		return true;
	}
	
	private boolean startInputConnection() throws IOException
	{
		inputStream = new NvController(hostAddr);
		inputStream.initialize();
		return true;
	}
	
	public void stop()
	{
		threadPool.shutdownNow();
		
		if (videoStream != null) {
			videoStream.abort();
		}
		if (audioStream != null) {
			audioStream.abort();
		}
		
		if (controlStream != null) {
			controlStream.abort();
		}
		
		if (inputStream != null) {
			inputStream.close();
			inputStream = null;
		}
	}
	
	private void displayMessage(final String text) {
		SwingUtilities.invokeLater(new Runnable() {
			@Override
			public void run() {
				JOptionPane.showMessageDialog(parent, text);
			}
		});
	}
}
