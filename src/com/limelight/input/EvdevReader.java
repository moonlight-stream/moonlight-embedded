package com.limelight.input;

import com.limelight.LimeLog;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public abstract class EvdevReader implements Runnable {
	
	private final static int EVIOCGNAME = 0x06;
	
	private InputStream in;
	private ByteBuffer inputBuffer;

	public EvdevReader(String device) throws FileNotFoundException, IOException {
		File file = new File(device);
		if (!file.exists())
			throw new FileNotFoundException("File " + device + " not found");
		if (!file.canRead())
			throw new IOException("Can't read from " + device);
		
		byte[] data = new byte[255];
		int request = IO.getRequest(IO.READ_ONLY, EvdevConstants.EVDEV_TYPE, EVIOCGNAME, data.length-1);
		if (!IO.ioctl(device, data, request))
			throw new IOException("Path " + device + " is not a evdev device");
			
		LimeLog.info("Using " + new String(data));
			
		in = new FileInputStream(file);
		inputBuffer = ByteBuffer.allocate(EvdevConstants.MAX_STRUCT_SIZE_BYTES);
		inputBuffer.order(ByteOrder.nativeOrder());
	}
	
	public void start() {
		Thread thread = new Thread(this);
		thread.setDaemon(true);
		thread.setName("Input - Receiver");
		thread.start();
	}
	
	protected abstract void parseEvent(ByteBuffer buffer);
	protected abstract void deviceRemoved();
	
	@Override
	public void run() {
		try {
			while (true) {
				inputBuffer.limit(in.read(inputBuffer.array()));
				parseEvent(inputBuffer);
				inputBuffer.clear();
			}
		} catch (IOException e) {
			LimeLog.warning("Input device removed");
			deviceRemoved();
		}
	}

}
