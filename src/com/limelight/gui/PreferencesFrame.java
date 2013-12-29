package com.limelight.gui;

import java.awt.Dimension;
import java.awt.Toolkit;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JPanel;

import com.limelight.settings.PreferencesManager;
import com.limelight.settings.PreferencesManager.Preferences;
import com.limelight.settings.PreferencesManager.Preferences.Resolution;

/**
 * A frame that holds user preferences such as streaming resolution
 * @author Diego Waxemberg
 */
public class PreferencesFrame extends JFrame {
	private static final long serialVersionUID = 1L;
	private JComboBox resolution;
	private JCheckBox fullscreen;
	private Preferences prefs;
	
	/**
	 * Construcs a new frame and loads the saved preferences.
	 * <br>The frame is not made visible until a call to <br>build()</br> is made.
	 */
	public PreferencesFrame() {
		super("Preferences");
		this.setSize(200, 100);
		this.setResizable(false);
		this.setAlwaysOnTop(true);
		prefs = PreferencesManager.getPreferences();
	}
	
	/**
	 * Constructs all components of the frame and makes the frame visible to the user.
	 */
	public void build() {
		
		JPanel mainPanel = new JPanel();
		mainPanel.setLayout(new BoxLayout(mainPanel, BoxLayout.Y_AXIS));
		
		resolution = new JComboBox();
		for (Resolution res : Resolution.values()) {
			resolution.addItem(res);
		}
		
		resolution.setSelectedItem(prefs.getResolution());
		
		fullscreen = new JCheckBox("Fullscreen");
		fullscreen.setSelected(prefs.getFullscreen());
	
		Box resolutionBox = Box.createHorizontalBox();
		resolutionBox.add(Box.createHorizontalGlue());
		resolutionBox.add(resolution);
		resolutionBox.add(Box.createHorizontalGlue());
		
		Box fullscreenBox = Box.createHorizontalBox();
		fullscreenBox.add(Box.createHorizontalGlue());
		fullscreenBox.add(fullscreen);
		fullscreenBox.add(Box.createHorizontalGlue());
		
		mainPanel.add(Box.createVerticalStrut(10));
		mainPanel.add(resolutionBox);
		mainPanel.add(Box.createVerticalStrut(5));
		mainPanel.add(fullscreenBox);
		mainPanel.add(Box.createVerticalGlue());
		
		this.addWindowListener(new WindowAdapter() {
			@Override
			public void windowClosing(WindowEvent e) {
				super.windowClosing(e);
				if (prefsChanged()) {
					writePreferences();
				}
			}
		});
		
		this.getContentPane().add(mainPanel);

		Dimension dim = Toolkit.getDefaultToolkit().getScreenSize();
		//center on screen
		this.setLocation((int)dim.getWidth()/2-this.getWidth()/2, (int)dim.getHeight()/2-this.getHeight()/2);
		
		this.setVisible(true);
	}
	
	/*
	 * Checks if the preferences have changed from the cached preferences.
	 */
	private boolean prefsChanged() {
		return (prefs.getResolution() != resolution.getSelectedItem()) ||
				(prefs.getFullscreen() != fullscreen.isSelected());
	}
	
	/*
	 * Writes the preferences to the disk.
	 */
	private void writePreferences() {
		prefs.setFullscreen(fullscreen.isSelected());
		prefs.setResolution((Resolution)resolution.getSelectedItem());
		PreferencesManager.writePreferences(prefs);
	}
	
}
