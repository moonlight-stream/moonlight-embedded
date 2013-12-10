package com.limelight.gui;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Container;
import java.awt.Cursor;
import java.awt.Dimension;
import java.awt.Frame;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.awt.Point;
import java.awt.Toolkit;
import java.awt.image.BufferedImage;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JFrame;
import javax.swing.JLabel;
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
	
	public void build(NvConnection conn, boolean fullscreen) {
		keyboard = new KeyboardHandler(conn);
		mouse = new MouseHandler(conn, this);

		this.addKeyListener(keyboard);
		this.addMouseListener(mouse);
		this.addMouseMotionListener(mouse);

		this.setFocusTraversalKeysEnabled(false);

		Dimension dim = Toolkit.getDefaultToolkit().getScreenSize();
		this.setSize(1280,720);
		
		//This might break if the screen res is too small...not sure though
		this.setLocation(dim.width/2-this.getSize().width/2, 0);

		this.setUndecorated(true);
		this.setBackground(Color.BLACK);
		
		if (fullscreen) {
			makeFullScreen();
			this.setLocation(getLocation().x, dim.height/2-this.getSize().height/2);
		}
		hideCursor();

		this.setVisible(true);
	}

	private void makeFullScreen() {
		
		this.setExtendedState(Frame.MAXIMIZED_BOTH);
		
		GraphicsDevice gd = GraphicsEnvironment.getLocalGraphicsEnvironment().getDefaultScreenDevice();
		if (gd.isFullScreenSupported()) {
			gd.setFullScreenWindow(this);
		}
	}

	private void hideCursor() {
		// Transparent 16 x 16 pixel cursor image.
		BufferedImage cursorImg = new BufferedImage(16, 16, BufferedImage.TYPE_INT_ARGB);

		// Create a new blank cursor.
		Cursor blankCursor = Toolkit.getDefaultToolkit().createCustomCursor(
				cursorImg, new Point(0, 0), "blank cursor");

		// Set the blank cursor to the JFrame.
		this.getContentPane().setCursor(blankCursor);
	}
	
	public void showSpinner(Stage stage) {
		
		if (spinner == null) {
			Container c = this.getContentPane();
			JPanel panel = new JPanel();
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
	
}
