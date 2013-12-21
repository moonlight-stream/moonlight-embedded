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
			Box componentBox = Box.createHorizontalBox();

			componentBox.add(Box.createHorizontalStrut(10));
			componentBox.add(components[i].getLabel());
			componentBox.add(Box.createHorizontalGlue());
			componentBox.add(components[i].getMapButton());
			componentBox.add(Box.createHorizontalStrut(5));
			componentBox.add(components[i].getInvertBox());
			componentBox.add(Box.createHorizontalStrut(5));
			componentBox.add(components[i].getTriggerBox());
			componentBox.add(Box.createHorizontalStrut(10));
			
			Dimension buttonSize = new Dimension(110,32);
			components[i].getMapButton().setMaximumSize(buttonSize);
			components[i].getMapButton().setMinimumSize(buttonSize);
			components[i].getMapButton().setPreferredSize(buttonSize);
			
			components[i].getMapButton().addActionListener(createListener());
			components[i].getMapButton().setText(config.getMapping(components[i]));

			components[i].getInvertBox().addItemListener(new ItemListener() {
				@Override
				public void itemStateChanged(ItemEvent e) {
					JCheckBox clicked = (JCheckBox)e.getItem();
					ControllerComponent contComp = ControllerComponent.valueOf(clicked.getName());
					contComp.invert(e.getStateChange() == ItemEvent.SELECTED);
					configChanged = true;
				}
			});
			
			components[i].getTriggerBox().addItemListener(new ItemListener() {
				@Override
				public void itemStateChanged(ItemEvent e) {
					JCheckBox clicked = (JCheckBox)e.getItem();
					ControllerComponent contComp = ControllerComponent.valueOf(clicked.getName());
					contComp.trigger(e.getStateChange() == ItemEvent.SELECTED);
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
					
					ControllerComponent oldConfig = config.getControllerComponent(newMapping);
					if (oldConfig != null) {
						config.removeMapping(newMapping);
						oldConfig.getMapButton().setText("");
					}

					config.insertMapping(contComp, newMapping);
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
