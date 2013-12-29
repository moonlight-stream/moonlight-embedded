package com.limelight.gui;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Container;
import java.awt.Cursor;
import java.awt.Dimension;
import java.awt.DisplayMode;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.awt.Point;
import java.awt.Toolkit;
import java.awt.image.BufferedImage;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JProgressBar;

import com.limelight.Limelight;
import com.limelight.input.KeyboardHandler;
import com.limelight.input.MouseHandler;
import com.limelight.nvstream.NvConnection;
import com.limelight.nvstream.NvConnectionListener.Stage;
import com.limelight.nvstream.StreamConfiguration;

/**
 * The frame to which the video is rendered
 * @author Diego Waxemberg
 * <br>Cameron Gutman
 *
 */
public class StreamFrame extends JFrame {
	private static final long serialVersionUID = 1L;
	
	private static final double DESIRED_ASPECT_RATIO = 16.0/9.0;
	private static final double ALTERNATE_ASPECT_RATIO = 16.0/10.0;

	private KeyboardHandler keyboard;
	private MouseHandler mouse;
	private JProgressBar spinner;
	private JLabel spinnerLabel;
	private Cursor noCursor;
	private Limelight limelight;

	/**
	 * Frees the mouse ie. makes it visible and allowed to move outside the frame.
	 */
	public void freeMouse() {
		mouse.free();
		showCursor();
	}

	/**
	 * Captures the mouse ie. makes it invisible and not allowed to leave the frame
	 */
	public void captureMouse() {
		mouse.capture();
		hideCursor();
	}

	/**
	 * Builds the components of this frame with the specified configurations.
	 * @param conn the connection this frame belongs to
	 * @param streamConfig the configurations for this frame
	 * @param fullscreen if the frame should be made fullscreen
	 */
	public void build(Limelight limelight, NvConnection conn, StreamConfiguration streamConfig, boolean fullscreen) {
		this.limelight = limelight;
		
		keyboard = new KeyboardHandler(conn, this);
		mouse = new MouseHandler(conn, this);

		this.addKeyListener(keyboard);
		this.addMouseListener(mouse);
		this.addMouseMotionListener(mouse);

		this.setFocusTraversalKeysEnabled(false);

		this.setSize(streamConfig.getWidth(), streamConfig.getHeight());
		
		this.setBackground(Color.BLACK);
		this.getContentPane().setBackground(Color.BLACK);
		this.getRootPane().setBackground(Color.BLACK);
		
		if (fullscreen) {
			makeFullScreen(streamConfig);
		}

		hideCursor();
		this.setVisible(true);
	}
	
	private ArrayList<DisplayMode> getDisplayModesByAspectRatio(DisplayMode[] configs, double aspectRatio) {
		ArrayList<DisplayMode> matchingConfigs = new ArrayList<DisplayMode>();
		
		for (DisplayMode config : configs) {
			if ((double)config.getWidth()/(double)config.getHeight() == aspectRatio) {
				matchingConfigs.add(config);
			}
		}
		
		return matchingConfigs;
	}
	
	private DisplayMode getBestDisplay(StreamConfiguration targetConfig, DisplayMode[] configs) {
		int targetDisplaySize = targetConfig.getWidth()*targetConfig.getHeight();
		
		// Try to match the target aspect ratio
		ArrayList<DisplayMode> aspectMatchingConfigs = getDisplayModesByAspectRatio(configs, DESIRED_ASPECT_RATIO);
		if (aspectMatchingConfigs.size() == 0) {
			// No matches for the target, so try the alternate
			aspectMatchingConfigs = getDisplayModesByAspectRatio(configs, ALTERNATE_ASPECT_RATIO);
			if (aspectMatchingConfigs.size() == 0) {
				// No matches for either, so just use all of them
				aspectMatchingConfigs = new ArrayList<DisplayMode>(Arrays.asList(configs));
			}
		}
		
		// Sort by display size
		Collections.sort(aspectMatchingConfigs, new Comparator<DisplayMode>() {
			@Override
			public int compare(DisplayMode o1, DisplayMode o2) {
				if (o1.getWidth()*o1.getHeight() > o2.getWidth()*o2.getHeight()) {
					return -1;
				} else if (o2.getWidth()*o2.getHeight() > o1.getWidth()*o1.getHeight()) {
					return 1;
				} else {
					return 0;
				}
			}
		});

		// Find the aspect-matching config with the closest matching display size
		DisplayMode bestConfig = null;
		for (DisplayMode config : aspectMatchingConfigs) {
			if (config.getWidth()*config.getHeight() >= targetDisplaySize) {
				bestConfig = config;
			}
		}
		
		if (bestConfig != null) {
			System.out.println("Using full-screen display mode "+bestConfig.getWidth()+"x"+bestConfig.getHeight()+
					" for "+targetConfig.getWidth()+"x"+targetConfig.getHeight()+" stream");
		}
		
		return bestConfig;
	}

	private void makeFullScreen(StreamConfiguration streamConfig) {
		GraphicsDevice gd = GraphicsEnvironment.getLocalGraphicsEnvironment().getDefaultScreenDevice();
		if (gd.isFullScreenSupported()) {
			this.setUndecorated(true);
			gd.setFullScreenWindow(this);
			
			if (gd.isDisplayChangeSupported()) {
				DisplayMode config = getBestDisplay(streamConfig, gd.getDisplayModes());
				if (config != null) {
					gd.setDisplayMode(config);
				}
			} else {
				JOptionPane.showMessageDialog(
						this,
						"Unable to change display resolution. \nThis may not be the correct resolution",
						"Display Resolution",
						JOptionPane.ERROR_MESSAGE);
			}
		} else {
			JOptionPane.showMessageDialog(
					this, 
					"Your operating system does not support fullscreen.", 
					"Fullscreen Unsupported",
					JOptionPane.ERROR_MESSAGE);
		}
	}
	
	/**
	 * Makes the mouse cursor invisible
	 */
	public void hideCursor() {
		if (noCursor == null) {
			// Transparent 16 x 16 pixel cursor image.
			BufferedImage cursorImg = new BufferedImage(16, 16, BufferedImage.TYPE_INT_ARGB);

			// Create a new blank cursor.
			noCursor = Toolkit.getDefaultToolkit().createCustomCursor(
					cursorImg, new Point(0, 0), "blank cursor");
		}
		// Set the blank cursor to the JFrame.
		this.setCursor(noCursor);
		this.getContentPane().setCursor(noCursor);
	}

	/**
	 * Makes the mouse cursor visible
	 */
	public void showCursor() {
		this.setCursor(Cursor.getDefaultCursor());
		this.getContentPane().setCursor(Cursor.getDefaultCursor());
	}

	/**
	 * Shows a progress bar with a label underneath that tells the user what 
	 * loading stage the stream is at.
	 * @param stage the currently loading stage
	 */
	public void showSpinner(Stage stage) {

		if (spinner == null) {
			Container c = this.getContentPane();
			JPanel panel = new JPanel();
			panel.setBackground(Color.BLACK);
			panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));

			spinner = new JProgressBar();
			spinner.setIndeterminate(true);
			spinner.setMaximumSize(new Dimension(150, 30));

			spinnerLabel = new JLabel();
			spinnerLabel.setForeground(Color.white);

			Box spinBox = Box.createHorizontalBox();
			spinBox.add(Box.createHorizontalGlue());
			spinBox.add(spinner);
			spinBox.add(Box.createHorizontalGlue());

			Box lblBox = Box.createHorizontalBox();
			lblBox.add(Box.createHorizontalGlue());
			lblBox.add(spinnerLabel);
			lblBox.add(Box.createHorizontalGlue());

			panel.add(Box.createVerticalGlue());
			panel.add(spinBox);
			panel.add(Box.createVerticalStrut(10));
			panel.add(lblBox);
			panel.add(Box.createVerticalGlue());

			c.setLayout(new BorderLayout());
			c.add(panel, "Center");
		}
		spinnerLabel.setText("Starting " + stage.getName() + "...");
	}

	/**
	 * Hides the spinner and the label
	 */
	public void hideSpinner() {
		spinner.setVisible(false);
		spinnerLabel.setVisible(false);
	}

	/**
	 * Stops the stream and destroys the frame
	 */
	public void close() {
		limelight.stop();
		dispose();
	}
}
