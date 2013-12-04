package com.limelight.gui;

import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JTextField;

import com.limelight.Limelight;

public class MainFrame {
	private JTextField hostField;
	private JButton pair;
	private JButton stream;
	
	public MainFrame() {
	}
	
	public void build() {
		JFrame limeFrame = new JFrame("Limelight V" + Limelight.VERSION);
		limeFrame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		Container mainPane = limeFrame.getContentPane();
		
		mainPane.setLayout(new BorderLayout());
		
		JPanel centerPane = new JPanel();
		centerPane.setLayout(new BoxLayout(centerPane, BoxLayout.Y_AXIS));
		
		hostField = new JTextField();
		hostField.setMaximumSize(new Dimension(Integer.MAX_VALUE, 24));
		hostField.setToolTipText("Enter host name or IP address");
		hostField.setText("GeForce PC host");
		
		stream = new JButton("Start Streaming");
		stream.addActionListener(createStreamButtonListener());
		stream.setToolTipText("Start the GeForce stream");
		
		pair = new JButton("Pair");
		pair.addActionListener(createPairButtonListener());
		pair.setToolTipText("Send pair request to GeForce PC");
		
		
		Box streamBox = Box.createHorizontalBox();
		streamBox.add(Box.createHorizontalGlue());
		streamBox.add(stream);
		streamBox.add(Box.createHorizontalGlue());
		
		Box pairBox = Box.createHorizontalBox();
		pairBox.add(Box.createHorizontalGlue());
		pairBox.add(pair);
		pairBox.add(Box.createHorizontalGlue());
		
		Box contentBox = Box.createVerticalBox();
		contentBox.add(Box.createVerticalStrut(20));
		contentBox.add(hostField);
		contentBox.add(Box.createVerticalStrut(10));
		contentBox.add(streamBox);
		contentBox.add(Box.createVerticalStrut(10));
		contentBox.add(pairBox);
		contentBox.add(Box.createVerticalGlue());
		
		
		centerPane.add(contentBox);
		mainPane.add(centerPane, "Center");
		
		limeFrame.setSize(1000, 800);
		limeFrame.setVisible(true);
		
	}
	
	private ActionListener createStreamButtonListener() {
		return new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				Limelight.createInstance(hostField.getText());
			}
		};
	}
	
	private ActionListener createPairButtonListener() {
		return null;
	}
}
