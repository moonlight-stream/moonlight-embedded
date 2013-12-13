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

<<<<<<< HEAD
=======
	private int centerX;
	private int centerY;
>>>>>>> 89e07e63ef3e007caef9aec3334bfc2c75aafd1c
	private KeyboardHandler keyboard;
	private MouseHandler mouse;
	private JProgressBar spinner;
	private JLabel spinnerLabel;
	private Cursor noCursor;
	private NvConnection conn;
<<<<<<< HEAD
	private static final int WIDTH = 1280;
	private static final int HEIGHT = 720;
	
	public void freeMouse() {
		mouse.free();
		showCursor();
	}

=======

	public StreamFrame() {
		Dimension dim = Toolkit.getDefaultToolkit().getScreenSize();
		this.setSize(1280,720);
		centerX = dim.width/2-this.getSize().width/2;
		centerY = dim.height/2-this.getSize().height/2;
	}

	public void freeMouse() {
		mouse.free();
		showCursor();
	}

>>>>>>> 89e07e63ef3e007caef9aec3334bfc2c75aafd1c
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
<<<<<<< HEAD
		this.getContentPane().setBackground(Color.BLACK);
		this.getRootPane().setBackground(Color.BLACK);
		
=======

>>>>>>> 89e07e63ef3e007caef9aec3334bfc2c75aafd1c
		if (fullscreen) {
			makeFullScreen();
		}

		hideCursor();
		this.setVisible(true);
	}

	private void makeFullScreen() {
		GraphicsDevice gd = GraphicsEnvironment.getLocalGraphicsEnvironment().getDefaultScreenDevice();
		if (gd.isFullScreenSupported()) {
			this.setUndecorated(true);
			gd.setFullScreenWindow(this);
			
			if (gd.isDisplayChangeSupported()) {
				DisplayMode[] configs = gd.getDisplayModes();
				for (DisplayMode config : configs) {
					if (config.getWidth() == 1280 && config.getHeight() == 720 ||
							config.getWidth() == 1280 && config.getHeight() == 800) {
						gd.setDisplayMode(config);
						break;
					}
				}
			} else {
				JOptionPane.showMessageDialog(
						this,
						"Unable to change display resolution. \nThis may not be the correct resolution",
						"Display Resolution",
						JOptionPane.INFORMATION_MESSAGE);
			}
		} else {
			JOptionPane.showMessageDialog(
					this, 
					"Your operating system does not support fullscreen.", 
					"Fullscreen Unsupported",
					JOptionPane.ERROR_MESSAGE);
		}
	}

<<<<<<< HEAD
	public void hideCursor() {
=======
	private void hideCursor() {
>>>>>>> 89e07e63ef3e007caef9aec3334bfc2c75aafd1c
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
<<<<<<< HEAD
	}

	public void showCursor() {
=======

	}

	private void showCursor() {
>>>>>>> 89e07e63ef3e007caef9aec3334bfc2c75aafd1c
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
