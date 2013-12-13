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
import java.util.Arrays;
import java.util.Comparator;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JProgressBar;

import com.limelight.input.KeyboardHandler;
import com.limelight.input.MouseHandler;
import com.limelight.nvstream.NvConnection;
import com.limelight.nvstream.NvConnectionListener.Stage;

public class StreamFrame extends JFrame {
	private static final long serialVersionUID = 1L;

	private KeyboardHandler keyboard;
	private MouseHandler mouse;
	private JProgressBar spinner;
	private JLabel spinnerLabel;
	private Cursor noCursor;
	private NvConnection conn;

	private static final int WIDTH = 1280;
	private static final int HEIGHT = 720;
	

	public void freeMouse() {
		mouse.free();
		showCursor();
	}

	public void captureMouse() {
		mouse.capture();
		hideCursor();
	}

	public void build(NvConnection conn, boolean fullscreen) {
		this.conn = conn;

		keyboard = new KeyboardHandler(conn, this);
		mouse = new MouseHandler(conn, this);

		this.addKeyListener(keyboard);
		this.addMouseListener(mouse);
		this.addMouseMotionListener(mouse);

		this.setFocusTraversalKeysEnabled(false);

		this.setSize(WIDTH, HEIGHT);
		
		this.setBackground(Color.BLACK);
		this.getContentPane().setBackground(Color.BLACK);
		this.getRootPane().setBackground(Color.BLACK);
		
		if (fullscreen) {
			makeFullScreen();
		}

		hideCursor();
		this.setVisible(true);
	}
	
	private DisplayMode getBestDisplay(DisplayMode[] configs) {
		Arrays.sort(configs, new Comparator<DisplayMode>() {
			@Override
			public int compare(DisplayMode o1, DisplayMode o2) {
				if (o1.getWidth() > o2.getWidth()) {
					return -1;
				} else if (o2.getWidth() > o1.getWidth()) {
					return 1;
				} else {
					return 0;
				}
			}
		});
		DisplayMode bestConfig = null;
		for (DisplayMode config : configs) {
			if (config.getWidth() >= 1280 && config.getHeight() >= 720) {
				bestConfig = config;
			}
		}
		return bestConfig;
	}

	private void makeFullScreen() {
		GraphicsDevice gd = GraphicsEnvironment.getLocalGraphicsEnvironment().getDefaultScreenDevice();
		if (gd.isFullScreenSupported()) {
			this.setUndecorated(true);
			gd.setFullScreenWindow(this);
			
			if (gd.isDisplayChangeSupported()) {
				DisplayMode config = getBestDisplay(gd.getDisplayModes());
				if (config != null) {
					gd.setDisplayMode(config);
				} else {
					JOptionPane.showMessageDialog(
							this,
							"Your display does not support a resolution greater than 1280x720",
							"No Supported Resolution",
							JOptionPane.ERROR_MESSAGE);
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

	public void showCursor() {
		this.setCursor(Cursor.getDefaultCursor());
		this.getContentPane().setCursor(Cursor.getDefaultCursor());
	}

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

	public void hideSpinner() {
		spinner.setVisible(false);
		spinnerLabel.setVisible(false);
	}

	public void close() {
		dispose();
		conn.stop();
	}
}
