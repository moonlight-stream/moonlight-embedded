package com.limelight.input;

import com.limelight.LimeLog;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.FileChannel;

public abstract class EvdevReader implements Runnable {
	
	private FileChannel deviceInput;
	private ByteBuffer inputBuffer;

	public EvdevReader(String device) throws FileNotFoundException, IOException {
		File file = new File(device);
		if (!file.exists())
			throw new FileNotFoundException("File " + device + " not found");
		if (!file.canRead())
			throw new IOException("Can't read from " + device);
			
		FileInputStream in = new FileInputStream(file);
        deviceInput = in.getChannel();
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
	
	@Override
	public void run() {
		try {
			while (true) {
				while(inputBuffer.remaining()==EvdevConstants.MAX_STRUCT_SIZE_BYTES)
					deviceInput.read(inputBuffer);

				inputBuffer.flip();
				parseEvent(inputBuffer);
				inputBuffer.clear();
			}
		} catch (IOException e) {
			LimeLog.warning("Input device removed");
		}
	}

}
