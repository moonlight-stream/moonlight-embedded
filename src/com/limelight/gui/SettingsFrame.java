package com.limelight.gui;

import java.awt.Container;
import java.awt.Dimension;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JTextField;

import net.java.games.input.Component;
import net.java.games.input.Event;
import net.java.games.input.EventQueue;

import com.limelight.input.ControllerComponent;
import com.limelight.input.Gamepad;
import com.limelight.input.GamepadHandler;
import com.limelight.input.GamepadSettings;
import com.limelight.settings.GamepadSettingsManager;

public class SettingsFrame extends JFrame {
	private static final long serialVersionUID = 1L;
	
	private boolean configChanged = false;
	private GamepadSettings config;
	
	public SettingsFrame() {
		super("Limelight Settings");
		this.setSize(800, 500);
		this.setResizable(false);
		this.setAlwaysOnTop(true);
	}
	
	public void build() {
		Container c = this.getContentPane();
		JPanel mainPanel = new JPanel();
		mainPanel.setLayout(new BoxLayout(mainPanel, BoxLayout.X_AXIS));
		
		Box leftColumn = Box.createVerticalBox();
		Box rightColumn = Box.createVerticalBox();
		
		leftColumn.add(Box.createVerticalStrut(10));
		rightColumn.add(Box.createVerticalStrut(10));
		
		
		ControllerComponent[] components = ControllerComponent.values();
		for (int i = 0; i < components.length; i++) {
			Box componentBox = Box.createHorizontalBox();
			
			componentBox.add(Box.createHorizontalStrut(10));
			componentBox.add(components[i].getLabel());
			componentBox.add(Box.createHorizontalGlue());
			componentBox.add(components[i].getMapButton());
			componentBox.add(Box.createHorizontalStrut(10));
			components[i].getMapButton().setMaximumSize(new Dimension(50, 30));
			components[i].getMapButton().addActionListener(createListener());
			if (i > components.length / 2) {
				rightColumn.add(componentBox);
				if (i < components.length - 1) {
					rightColumn.add(Box.createVerticalStrut(5));
				}
			} else {
				leftColumn.add(componentBox);
				if (i < components.length / 2 - 1) {
					leftColumn.add(Box.createVerticalStrut(5));
				}
			}
		}
		
		rightColumn.add(Box.createVerticalGlue());
		leftColumn.add(Box.createVerticalGlue());
		
		mainPanel.add(Box.createHorizontalStrut(20));
		mainPanel.add(leftColumn);
		mainPanel.add(Box.createHorizontalGlue());
		mainPanel.add(rightColumn);
		
		mainPanel.add(Box.createHorizontalStrut(20));
		
		c.add(mainPanel);
		
		this.addWindowListener(new WindowAdapter() {
			@Override
			public void windowClosing(WindowEvent e) {
				super.windowClosing(e);
				if (configChanged) {
					updateConfigs();
				}
			}
		});
		
		Dimension dim = Toolkit.getDefaultToolkit().getScreenSize();
		this.setLocation(dim.width/2-this.getSize().width/2, dim.height/2-this.getSize().height/2);
		this.setVisible(true);
	}
	
	private ActionListener createListener() {
		return new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				//#allthejank
				ControllerComponent contComp = ControllerComponent.valueOf(((JTextField)e.getSource()).getName());
				
				contComp.getMapButton().setText("Select Input");
				
				Gamepad listenPad = GamepadHandler.getGamepads().get(0);
				listenPad.poll();
				EventQueue queue = listenPad.getEvents();
				Event event = new Event();
				queue.getNextEvent(event);
				Component comp = event.getComponent();
				contComp.getMapButton().setText(comp.getName());
				
				config = listenPad.getConfiguration();
				if (config == null) {
					config = new GamepadSettings();
				}
				
				config.insertSetting(contComp, comp);
				
				configChanged = true;
			}
		};
	}
	
	private void updateConfigs() {
		GamepadSettingsManager.writeSettings(config);
	}
}
