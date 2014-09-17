package com.limelight;

import com.limelight.binding.PlatformBinding;
import com.limelight.binding.audio.FakeAudioRenderer;
import com.limelight.binding.video.FakeVideoRenderer;
import com.limelight.input.EvdevLoader;
import com.limelight.input.GamepadMapper;
import com.limelight.input.GamepadMapping;
import com.limelight.nvstream.NvConnection;
import com.limelight.nvstream.NvConnectionListener;
import com.limelight.nvstream.StreamConfiguration;
import com.limelight.nvstream.av.video.VideoDecoderRenderer;
import com.limelight.nvstream.http.NvApp;
import com.limelight.nvstream.http.NvHTTP;
import com.limelight.nvstream.http.PairingManager;
import com.limelight.nvstream.mdns.MdnsComputer;
import com.limelight.nvstream.mdns.MdnsDiscoveryAgent;
import com.limelight.nvstream.mdns.MdnsDiscoveryListener;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * Main class for Limelight Embedded
 * @author Diego Waxemberg<br>
 * Cameron Gutman
 * Iwan Timmer
 */
public class Limelight implements NvConnectionListener {

	private InetAddress host;
	private NvConnection conn;
	private boolean connectionTerminating;
	private Logger logger;

	public Limelight() {
	}
	
	/**
	 * Constructs a new instance based on the given host
	 * @param host can be hostname or IP address.
	 */
	public Limelight(InetAddress host) {
		this.host = host;
	}
	
	/*
	 * Creates a connection to the host and starts up the stream.
	 */
	private void startUp(StreamConfiguration streamConfig, List<String> inputs, String mappingFile, String audioDevice, boolean tests) {
		if (tests) {
			boolean test = true;
			String vm = System.getProperties().getProperty("java.vm.name");
			if (!vm.contains("HotSpot")) {
				System.err.println("You are using a unsupported VM: " + vm);
				System.err.println("Please update to Oracle Java (Embedded) for better performances");
				test = false;
			}
			String display = System.getenv("DISPLAY");
			if (display!=null) {
				System.err.println("X server is propably running");
				System.err.println("Please exit the X server for a lower latency");
				test = false;
			}
			
			if (!test) {
				System.err.println("Fix problems or start application with parameter -notest");
				return;
			}
		}
	
		conn = new NvConnection(host.getHostAddress(), getUniqueId(), this, streamConfig, PlatformBinding.getCryptoProvider());
		
		GamepadMapping mapping = null;
		if (mappingFile!=null) {
			try {
				mapping = new GamepadMapping(new File(mappingFile));
			} catch (IOException e) {
				displayError("Mapping", "Can't load gamepad mapping from " + mappingFile);
				System.exit(3);
			}
		} else
			mapping = new GamepadMapping();
		
		try {
			new EvdevLoader(conn, mapping, inputs).start();
		} catch (FileNotFoundException ex) {
			displayError("Input", "Input could not be found");
			return;
		} catch (IOException ex) {
			displayError("Input", "No input could be readed");
			displayError("Input", "Try to run as root");
			return;
		}

		conn.start(PlatformBinding.getDeviceName(), null,
				VideoDecoderRenderer.FLAG_PREFER_QUALITY,
				PlatformBinding.getAudioRenderer(audioDevice),
				PlatformBinding.getVideoDecoderRenderer());
	}
	
	/*
	 * Creates a connection to the host and starts up the stream.
	 */
	private void startUpFake(StreamConfiguration streamConfig, String videoFile) {
		conn = new NvConnection(host.getHostAddress(), getUniqueId(), this, streamConfig, PlatformBinding.getCryptoProvider());
		conn.start(PlatformBinding.getDeviceName(), null,
				VideoDecoderRenderer.FLAG_PREFER_QUALITY,
				new FakeAudioRenderer(),
				new FakeVideoRenderer(videoFile));
	}
	
	/**
	 * Pair the device with the host
	 */
	private void pair() {
		NvHTTP httpConn;
	
		httpConn = new NvHTTP(host,
			getUniqueId(), PlatformBinding.getDeviceName(), PlatformBinding.getCryptoProvider());
		try {
			if (httpConn.getPairState() == PairingManager.PairState.PAIRED) {
				displayError("pair", "Already paired");
			} else {
				final String pinStr = PairingManager.generatePinString();

				displayMessage("Please enter the following PIN on the target PC: "+pinStr);

				PairingManager.PairState pairState = httpConn.pair(pinStr);
				if (pairState == PairingManager.PairState.PIN_WRONG) {
					displayError("pair", "Incorrect PIN");
				}
				else if (pairState == PairingManager.PairState.FAILED) {
					displayError("pair", "Pairing failed");
				}
				else if (pairState == PairingManager.PairState.PAIRED) {
					displayError("pair", "Paired successfully");
				}
			}
		} catch (Exception e) {
			displayError("Pair", e.getMessage());
		}
	}
	
	private void listApps() {
		NvHTTP conn = new NvHTTP(host, getUniqueId(), PlatformBinding.getDeviceName(), PlatformBinding.getCryptoProvider());
		displayMessage("Search apps");
		try {
			List<NvApp> apps = conn.getAppList();
			for (NvApp app:apps) {
				displayMessage(" " + app.getAppName() + (app.getIsRunning()?" (running)":""));
			}
		} catch (Exception e) {
			displayError("List", e.getMessage());
		}
	}
	
	/**
	 * The entry point for the application. <br>
	 * Does some initializations and then creates the main frame.
	 * @param args unused.
	 */
	public static void main(String args[]) {
		InetAddress host = null;
		List<String> inputs = new ArrayList<String>();
		int width = 1280;
		int height = 720;
		int refresh = 60;
		int bitrate = 10000;
		boolean parse = true;
		boolean tests = true;
		boolean sops = true;
		String mapping = null;
		String app = "Steam";
		String audio = "sysdefault";
		String video = null;
		String action = null;
		String out= null;
		Level debug = Level.SEVERE;
		
		for (int i = 0; i < args.length; i++) {
			if (args[i].equals("-input")) {
				if (i + 1 < args.length) {
					inputs.add(args[i+1]);
					i++;
				} else {
					System.out.println("Syntax error: input device expected after -input");
					System.exit(3);
				}
			} else if (args[i].equals("-mapping")) {
				if (i + 1 < args.length) {
					mapping = args[i+1];
					i++;
				} else {
					System.out.println("Syntax error: mapping file expected after -mapping");
					System.exit(3);
				}
			} else if (args[i].equals("-audio")) {
				if (i + 1 < args.length) {
					audio = args[i+1];
					i++;
				} else {
					System.out.println("Syntax error: audio device expected after -audio");
					System.exit(3);
				}
			} else if (args[i].equals("-720")) {
				height = 720;
				width = 1280;
			} else if (args[i].equals("-1080")) {
				height = 1080;
				width = 1920;
			} else if (args[i].equals("-width")) {
				if (i + 1 < args.length) {
					try {
						width = Integer.parseInt(args[i+1]);
					} catch (NumberFormatException e) {
						System.out.println("Syntax error: width must be a number");
						System.exit(3);
					}
					i++;
				} else {
					System.out.println("Syntax error: width expected after -width");
					System.exit(3);
				}
			} else if (args[i].equals("-height")) {
				if (i + 1 < args.length) {
					try {
						height = Integer.parseInt(args[i+1]);
					} catch (NumberFormatException e) {
						System.out.println("Syntax error: height must be a number");
						System.exit(3);
					}
					i++;
				} else {
					System.out.println("Syntax error: height expected after -height");
					System.exit(3);
				}
			} else if (args[i].equals("-30fps")) {
				refresh = 30;
			} else if (args[i].equals("-60fps")) {
				refresh = 60;
			} else if (args[i].equals("-bitrate")) {
				if (i + 1 < args.length) {
					try {
						bitrate = Integer.parseInt(args[i+1]);
					} catch (NumberFormatException e) {
						System.out.println("Syntax error: bitrate must be a number");
						System.exit(3);
					}
					i++;
				} else {
					System.out.println("Syntax error: bitrate expected after -bitrate");
					System.exit(3);
				}
			} else if (args[i].equals("-out")) {
				if (i + 1 < args.length) {
					video = args[i+1];
					i++;
				} else {
					System.out.println("Syntax error: output file expected after -out");
					System.exit(3);
				}
			} else if (args[i].equals("-app")) {
				if (i + 1 < args.length) {
					app = args[i+1];
					i++;
				} else {
					System.out.println("Syntax error: application name expected after -app");
					System.exit(3);
				}
			} else if (args[i].equals("-notest")) {
				tests = false;
			} else if (args[i].equals("-nosops")) {
				sops = false;
			} else if (args[i].equals("-v")) {
				debug = Level.WARNING;
			} else if (args[i].equals("-vv")) {
				debug = Level.ALL;
			} else if (args[i].startsWith("-")) {
				System.out.println("Syntax Error: Unrecognized argument: " + args[i]);
				parse = false;
			} else if (action == null) {
				action = args[i].toLowerCase();
				if (!action.equals("stream") && !action.equals("pair") && !action.equals("fake") && !action.equals("help") && !action.equals("discover") && !action.equals("list") && !action.equals("map")) {
					System.out.println("Syntax error: invalid action specified");
					System.exit(3);
				}
			} else if (action.equals("map") && out == null) {
				out = args[i];
			} else if (!action.equals("map") && host == null) {
				try {
					host = InetAddress.getByName(args[i]);
				} catch (UnknownHostException ex) {
					System.out.println("Failed to resolve host");
					System.exit(3);
				}
			} else {
				System.out.println("Syntax Error: Unrecognized argument: " + args[i]);
				parse = false;
			}
		}
		
		if (action == null) {
			System.out.println("Syntax Error: Missing required action argument");
			parse = false;
		} else if (action.equals("map")) {
			if (inputs.size() != 1) {
				System.out.println("Syntax error: specify one -input");
				parse = false;
			} else if (out == null) {
				System.out.println("Syntax error: specify the output file");
				parse = false;
			}
			
			if (parse) {
				try {
					GamepadMapper mapper = new GamepadMapper(inputs.get(0));
					mapper.setup();
					mapper.save(new File(out));
				} catch (IOException | InterruptedException ex) {
					System.err.println(ex.getMessage());
				}
				return;
			}
		} else if (action.equals("help"))
			parse = false;
		
		if (args.length == 0 || !parse) {
			System.out.println("Usage: java -jar limelight.jar action [options] host/file");
			System.out.println();
			System.out.println(" Actions:");
			System.out.println();
			System.out.println("\tmap\t\t\tCreate mapping file for gamepad");
			System.out.println("\tpair\t\t\tPair device with computer");
			System.out.println("\tstream\t\t\tStream computer to device");
			System.out.println("\tdiscover\t\tList available computers");
			System.out.println("\tlist\t\t\tList available games and applications");
			System.out.println("\thelp\t\t\tShow this help");
			System.out.println();
			System.out.println(" Mapping options:");
			System.out.println();
			System.out.println("\t-input <device>\t\tUse <device> as input");
			System.out.println();
			System.out.println(" Streaming options:");
			System.out.println();
			System.out.println("\t-720\t\t\tUse 1280x720 resolution [default]");
			System.out.println("\t-1080\t\t\tUse 1920x1080 resolution");
			System.out.println("\t-width <width>\t\tHorizontal resolution (default 1280)");
			System.out.println("\t-height <height>\tVertical resolution (default 720)");
			System.out.println("\t-30fps\t\t\tUse 30fps");
			System.out.println("\t-60fps\t\t\tUse 60fps [default]");
			System.out.println("\t-bitrate <bitrate>\tSpecify the bitrate in Kbps");
			System.out.println("\t-app <app>\t\tName of app to stream");
			System.out.println("\t-nosops\t\t\tDon't allow GFE to modify game settings");
			System.out.println("\t-input <device>\t\tUse <device> as input. Can be used multiple times");
			System.out.println("\t\t\t\t[default uses all devices in /dev/input]");
			System.out.println("\t-mapping <file>\t\tUse <file> as gamepad mapping configuration file");
			System.out.println("\t-audio <device>\t\tUse <device> as ALSA audio output device (default sysdefault)");
			System.out.println();
			System.out.println("Use ctrl-c to exit application");
			System.exit(5);
		}
		
		Limelight limelight;
		if (host == null || action.equals("discover")) {
			limelight = new Limelight();
		} else
			limelight = new Limelight(host);
		
		//Set debugging level
		limelight.setLevel(debug);
		
		if (limelight.host == null) {
			limelight.discover(!action.equals("discover"));
		}
		
		if (action.equals("stream") || action.equals("fake")) {
			StreamConfiguration streamConfig = new StreamConfiguration(app, width, height, refresh, bitrate, sops);
			
			if (action.equals("fake"))
				limelight.startUpFake(streamConfig, video);
			else
				limelight.startUp(streamConfig, inputs, mapping, audio, tests);
		} else if (action.equals("pair"))
			limelight.pair();
		else if (action.equals("list"))
			limelight.listApps();
	}
	
	public void discover(final boolean first) {
		displayMessage("Discovering GeForce PCs...");
		final Object mutex = new Object();
		MdnsDiscoveryAgent agent = new MdnsDiscoveryAgent(new MdnsDiscoveryListener() {
			@Override
			public void notifyComputerAdded(MdnsComputer computer) {
				displayMessage(" " + computer.getName() + " " + computer.getAddress().getHostAddress());
				host = computer.getAddress();
				if (first)
					synchronized (mutex) {
						mutex.notify();
					}
			}

			@Override
			public void notifyComputerRemoved(MdnsComputer computer) {
			}

			@Override
			public void notifyDiscoveryFailure(Exception e) {
			}
		});
		agent.startDiscovery(1000);
		synchronized (mutex) {
			try {
				mutex.wait();
			} catch (InterruptedException ex) { }
		}
		agent.stopDiscovery();
	}
	
	public String getUniqueId() {
		try {
			File file = new File("uniqueid.dat");
			if (file.exists()) {
				FileInputStream in = new FileInputStream(file);
				byte[] id = new byte[16];
				in.read(id);
				in.close();
				return new String(id);
			} else {
				String id = String.format("%016x", new Random().nextLong());
				FileOutputStream out = new FileOutputStream(file);
				out.write(id.getBytes());
				out.close();
				return id;
			}
		} catch (IOException ex) {
			LimeLog.severe(ex.getMessage());
		}
		
		return "limelight";
	}

	public void setLevel(Level level) {
		if (logger==null)
			logger = Logger.getLogger(LimeLog.class.getName());

		logger.setLevel(level);
	}

	public void stop() {
		connectionTerminating = true;
		conn.stop();
	}

	/**
	 * Callback to specify which stage is starting. Used to update UI.
	 * @param stage the Stage that is starting
	 */
	@Override
	public void stageStarting(Stage stage) {
		System.out.println("Starting "+stage.getName());
	}

	/**
	 * Callback that a stage has finished loading.
	 * <br><b>NOTE: Currently unimplemented.</b>
	 * @param stage the Stage that has finished.
	 */
	@Override
	public void stageComplete(Stage stage) {
	}

	/**
	 * Callback that a stage has failed. Used to inform user that an error occurred.
	 * @param stage the Stage that was loading when the error occurred
	 */
	@Override
	public void stageFailed(Stage stage) {
		conn.stop();
		displayError("Connection Error", "Starting " + stage.getName() + " failed");
	}

	/**
	 * Callback that the connection has finished loading and is started.
	 */
	@Override
	public void connectionStarted() {
	}

	/**
	 * Callback that the connection has been terminated for some reason.
	 * <br>This is were the stream shutdown procedure takes place.
	 * @param e the Exception that was thrown- probable cause of termination.
	 */
	@Override
	public void connectionTerminated(Exception e) {
		if (!(e instanceof InterruptedException)) {
			e.printStackTrace();
		}
		if (!connectionTerminating) {
			connectionTerminating = true;

			// Kill the connection to the target
			conn.stop();

			// Spin off a new thread to update the UI since
			// this thread has been interrupted and will terminate
			// shortly
			new Thread(new Runnable() {
				@Override
				public void run() {
					displayError("Connection Terminated", "The connection failed unexpectedly");
				}
			}).start();
		}
	}

	/**
	 * Displays a message to the user in the form of an info dialog.
	 * @param message the message to show the user
	 */
	@Override
	public void displayMessage(String message) {
		System.out.println(message);
	}	

	/**
	 * Displays an error to the user in the form of an error dialog
	 * @param title the title for the dialog frame
	 * @param message the message to show the user
	 */
	public void displayError(String title, String message) {
		System.err.println(title + " " + message);
	}

	@Override
	public void displayTransientMessage(String message) {
		displayMessage(message);
	}
}

