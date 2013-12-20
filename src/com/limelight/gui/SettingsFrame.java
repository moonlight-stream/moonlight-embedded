package com.limelight.gui;

import java.awt.Container;
import java.awt.Dimension;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.List;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JPanel;

import net.java.games.input.Component;
import net.java.games.input.Event;
import net.java.games.input.EventQueue;

import com.limelight.input.ControllerComponent;
import com.limelight.input.ControllerListener;
import com.limelight.input.Gamepad;
import com.limelight.input.GamepadHandler;
import com.limelight.input.GamepadMapping;
import com.limelight.settings.GamepadSettingsManager;

public class SettingsFrame extends JFrame {
	private static final long serialVersionUID = 1L;
	
	private boolean configChanged = false;
	private boolean shouldStartHandler = false;
	
	private GamepadMapping config;
	
	public SettingsFrame() {
		super("Gamepad Settings");
		System.out.println("Creating Settings Frame");
		this.setSize(500, 500);
		this.setResizable(false);
		this.setAlwaysOnTop(true);
		config = GamepadSettingsManager.getSettings();
		
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
			
			Dimension buttonSize = new Dimension(50,30);
			components[i].getMapButton().setMaximumSize(buttonSize);
			components[i].getMapButton().setMinimumSize(buttonSize);
			components[i].getMapButton().setPreferredSize(buttonSize);
			components[i].getMapButton().addActionListener(createListener());
			components[i].getMapButton().setText(config.getMapping(components[i]));
			
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
					ControllerListener.startUp();
				}
				if (shouldStartHandler) {
					GamepadHandler.startUp();
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
				ControllerComponent contComp = ControllerComponent.valueOf(((JButton)e.getSource()).getName());
				
				List<Gamepad> gamepads = GamepadHandler.getGamepads();
				
				if (gamepads.isEmpty()) {
					JOptionPane.showMessageDialog(SettingsFrame.this, "No Gamepad Detected");
					return;
				}
				
				contComp.getMapButton().setText("Select Input");
				
				ControllerListener.stopListening();
				
				if (GamepadHandler.isRunning()) {
					GamepadHandler.stopHandler();
					shouldStartHandler = true;
				}
				
				final Gamepad listenPad = gamepads.get(0);
				
				Component newMapping = null;
				
				while (newMapping == null) {
					listenPad.poll();
					EventQueue queue = listenPad.getEvents();
					Event event = new Event();
				
					while (queue.getNextEvent(event)) {
						if (Math.abs(event.getValue()) > .75F) {
							newMapping = event.getComponent();
							break;
						}
					}
				}
				
				//spin off a new thread to handle any other events we got
				new Thread(new Runnable() {
					@Override
					public void run() {
						listenPad.poll();
						listenPad.handleEvents(null);
					}
				}).start();
				
				ControllerComponent oldConfig = config.getControllerComponent(newMapping);
				if (oldConfig != null) {
					oldConfig.getMapButton().setText("");
				}
				config.insertMapping(contComp, newMapping);
				contComp.getMapButton().setText(newMapping.getName());
				configChanged = true;
			}
		};
	}
	
	private void updateConfigs() {
		GamepadSettingsManager.writeSettings(config);
	}
}
