package com.limelight.binding.audio;

import java.nio.ByteBuffer;
import java.nio.ShortBuffer;
import java.util.LinkedList;

import com.limelight.nvstream.av.ShortBufferDescriptor;

public class SoundBuffer {
	
	private LinkedList<ShortBufferDescriptor> bufferList;
	private int maxBuffers;
	
	public SoundBuffer(int maxBuffers) {
		this.bufferList = new LinkedList<ShortBufferDescriptor>();
		this.maxBuffers = maxBuffers;
	}
	
	public void queue(ShortBufferDescriptor buff) {
		if (bufferList.size() > maxBuffers) {
			bufferList.removeFirst();
		}
		
		bufferList.addLast(buff);
	}
	
	public int size() {
		int size = 0;
		for (ShortBufferDescriptor desc : bufferList) {
			size += desc.length;
		}
		return size;
	}
	
	public int fill(byte[] data, int offset, int length) {
		int filled = 0;
		
		// Convert offset and length to be relative to shorts
		offset /= 2;
		length /= 2;
		
		ShortBuffer sb = ByteBuffer.wrap(data).asShortBuffer();
		sb.position(offset);
		while (length > 0 && !bufferList.isEmpty()) {
			ShortBufferDescriptor buff = bufferList.getFirst();
			
			if (buff.length > length) {
				break;
			}
			
			sb.put(buff.data, buff.offset, buff.length);
			length -= buff.length;
			filled += buff.length;
			
			bufferList.removeFirst();
		}
		
		// Return bytes instead of shorts
		return filled * 2;
	}
}
