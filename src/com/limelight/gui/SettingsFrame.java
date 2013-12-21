package com.limelight.gui;

import java.awt.Container;
import java.awt.Dimension;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.List;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JCheckBox;
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
import com.limelight.input.GamepadMapping.Mapping;
import com.limelight.settings.GamepadSettingsManager;

public class SettingsFrame extends JFrame {
	private static final long serialVersionUID = 1L;

	private boolean configChanged = false;
	private boolean shouldStartHandler = false;

	private Thread mappingThread;
	private GamepadMapping config;

	public SettingsFrame() {
		super("Gamepad Settings");
		System.out.println("Creating Settings Frame");
		this.setSize(900, 500);
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
			Mapping mapping = config.getMappedComponent(components[i]);
			ControllerComponent comp = null;
			if (mapping == null) {
				comp = components[i];
			} else {
				comp = mapping.contComp;
			}
			Box componentBox = Box.createHorizontalBox();

			componentBox.add(Box.createHorizontalStrut(10));
			componentBox.add(comp.getLabel());
			componentBox.add(Box.createHorizontalGlue());
			componentBox.add(comp.getMapButton());
			componentBox.add(Box.createHorizontalStrut(5));
			componentBox.add(comp.getInvertBox());
			componentBox.add(Box.createHorizontalStrut(5));
			componentBox.add(comp.getTriggerBox());
			componentBox.add(Box.createHorizontalStrut(10));
			
			Dimension buttonSize = new Dimension(110,32);
			comp.getMapButton().setMaximumSize(buttonSize);
			comp.getMapButton().setMinimumSize(buttonSize);
			comp.getMapButton().setPreferredSize(buttonSize);
			
			comp.getMapButton().addActionListener(createListener());
			comp.getMapButton().setText(config.getMapping(comp));

			if (mapping != null) {
				comp.getInvertBox().setSelected(mapping.invert);
				comp.getTriggerBox().setSelected(mapping.trigger);
			}
			
			comp.getInvertBox().addItemListener(new ItemListener() {
				@Override
				public void itemStateChanged(ItemEvent e) {
					JCheckBox clicked = (JCheckBox)e.getItem();
					ControllerComponent contComp = ControllerComponent.valueOf(clicked.getName());
					config.getMappedComponent(contComp).invert = (e.getStateChange() == ItemEvent.SELECTED);
					configChanged = true;
				}
			});
			
			comp.getTriggerBox().addItemListener(new ItemListener() {
				@Override
				public void itemStateChanged(ItemEvent e) {
					JCheckBox clicked = (JCheckBox)e.getItem();
					ControllerComponent contComp = ControllerComponent.valueOf(clicked.getName());
					config.getMappedComponent(contComp).trigger = (e.getStateChange() == ItemEvent.SELECTED);
					configChanged = true;
				}
			});
			
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

		mainPanel.add(Box.createHorizontalStrut(10));
		mainPanel.add(leftColumn);
		mainPanel.add(Box.createHorizontalStrut(75));
		mainPanel.add(rightColumn);
		mainPanel.add(Box.createHorizontalStrut(10));

		c.add(mainPanel);

		this.addWindowListener(new WindowAdapter() {
			@Override
			public void windowClosing(WindowEvent e) {
				super.windowClosing(e);
				if (mappingThread != null && mappingThread.isAlive()) {
					mappingThread.interrupt();
				}
				if (configChanged) {
					updateConfigs();
					ControllerListener.startUp();
				}
				if (shouldStartHandler) {
					GamepadHandler.startUp();
				}
				dispose();
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
				map(contComp, gamepads.get(0));
			}
		};
	}

	private void map(final ControllerComponent contComp, final Gamepad pad) {
		if (mappingThread == null || !mappingThread.isAlive()) {
			contComp.getMapButton().setSelected(true);
			ControllerListener.stopListening();

			if (GamepadHandler.isRunning()) {
				GamepadHandler.stopHandler();
				shouldStartHandler = true;
			}

			contComp.getMapButton().setText("Select Input");

			mappingThread = new Thread(new Runnable() {
				@Override
				public void run() {
					Component newMapping = null;

					while (newMapping == null) {
						pad.poll();
						EventQueue queue = pad.getEvents();
						Event event = new Event();

						while (queue.getNextEvent(event)) {
							if (Math.abs(event.getValue()) > .75F) {
								newMapping = event.getComponent();
								break;
							}
						}
						
						try {
							Thread.sleep(100);
						} catch (InterruptedException e) {}
					}
					
					// start a new thread to go through all of the remaining events
					Thread consumeEvents = new Thread(new Runnable() {
						@Override
						public void run() {
							pad.poll();
							EventQueue queue = pad.getEvents();
							Event event = new Event();

							while (queue.getNextEvent(event)) {
							}
						}
					});
					consumeEvents.setName("Consume Events Thread");
					consumeEvents.start();
					
					Mapping oldConfig = config.getMapping(newMapping);
					if (oldConfig != null) {
						config.removeMapping(newMapping);
						oldConfig.contComp.getMapButton().setText("");
					}
					
					Mapping newConfig = config.getMappedComponent(contComp);
					if (newConfig == null) {
						newConfig = config.new Mapping(contComp, false, false);
					}
					config.insertMapping(newConfig, newMapping);
					contComp.getMapButton().setText(newMapping.getName());
					configChanged = true;
					contComp.getMapButton().setSelected(false);
				}
			});
			mappingThread.setName("Gamepad Mapping Thread");
			mappingThread.start();
		}

	}

	private void updateConfigs() {
		GamepadSettingsManager.writeSettings(config);
	}
}
