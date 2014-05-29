package com.limelight.input;

import com.limelight.LimeLog;
import com.limelight.nvstream.NvConnection;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FilenameFilter;
import java.io.IOException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardWatchEventKinds;
import java.nio.file.WatchEvent;
import java.nio.file.WatchKey;
import java.nio.file.WatchService;
import java.util.List;

/**
 * Class that handles input devices and a new ones when connected
 * @author Iwan Timmer
 */
public class EvdevLoader implements Runnable {
	
	private NvConnection conn;
	
	private GamepadMapping mapping;
	
	private List<String> inputs;
	
	private File input;
	private FilenameFilter filter;
	
	public EvdevLoader(NvConnection conn, GamepadMapping mapping, List<String> inputs) {
		this.conn = conn;
		this.mapping = mapping;
		this.inputs = inputs;
		this.input = new File("/dev/input");
		this.filter = new FilenameFilter() {
			@Override
			public boolean accept(File dir, String name) {
				return name.startsWith("event");
			}
		};
	}
	
	public void start() throws FileNotFoundException, IOException {
		boolean allInputs = inputs.isEmpty();
		
		if (allInputs) {
			String[] events = input.list(filter);
			
			for (String event:events)
				inputs.add(new File(input, event).getAbsolutePath());
		}
		
		boolean hasInput = false;
		IOException ex = null;
		for (String input:inputs) {
			try {
				EvdevHandler handler = new EvdevHandler(conn, input, mapping);
				handler.start();
				hasInput = true;
			} catch (FileNotFoundException e) {
				throw e;
			} catch (IOException e) {
				//Throw always exception for explicitly loaded inputs
				if (!allInputs)
					throw e;
				else
					ex = e;
			}
		}
		
		//Only throw exception when no input get loaded
		if (!hasInput && ex != null)
			throw ex;
		
		if (allInputs) {
			Thread thread = new Thread(this);
			thread.setDaemon(true);
			thread.setName("Input - Search");
			thread.start();
		}
	}
	
	@Override
	public void run() {
		try {
			Path evdev = Paths.get("/dev/input");
			
			WatchService watcher = evdev.getFileSystem().newWatchService();
			evdev.register(watcher, StandardWatchEventKinds.ENTRY_CREATE);
			
			WatchKey watckKey = watcher.take();
			List<WatchEvent<?>> events = watckKey.pollEvents();
			for (WatchEvent event:events) {
				if (event.kind() == StandardWatchEventKinds.ENTRY_CREATE) {
					String name = event.context().toString();
					if (filter.accept(input, name)) {
						LimeLog.info("Input " + name + " added");
						new EvdevHandler(conn, new File(input, name).getAbsolutePath(), mapping).start();
					}
				}
			}
		} catch (IOException | InterruptedException ex) {
			LimeLog.severe(ex.getMessage());
		}
	}
	
}
