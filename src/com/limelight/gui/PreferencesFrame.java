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

public class PreferencesFrame extends JFrame {
	private static final long serialVersionUID = 1L;
	private JComboBox resolution;
	private JCheckBox fullscreen;
	private Preferences prefs;
	private boolean prefsChanged = false;
	
	public PreferencesFrame() {
		super("Preferences");
		this.setSize(200, 100);
		this.setResizable(false);
		this.setAlwaysOnTop(true);
	}
	
	public void build() {
		
		JPanel mainPanel = new JPanel();
		mainPanel.setLayout(new BoxLayout(mainPanel, BoxLayout.Y_AXIS));
		
		resolution = new JComboBox();
		resolution.addItem("1080p");
		resolution.addItem("720p");
		
		fullscreen = new JCheckBox("Fullscreen");
		
		
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
				if (prefsChanged) {
					writePreferences();
				}
			}
		});
		
		this.getContentPane().add(mainPanel);

		Dimension dim = Toolkit.getDefaultToolkit().getScreenSize();
		//center on screen
		this.setLocation((int)dim.getWidth()/2-this.getWidth(), (int)dim.getHeight()/2-this.getHeight());
		
		this.setVisible(true);
	}
	
	private void writePreferences() {
		PreferencesManager.writePreferences(prefs);
	}
	
}
