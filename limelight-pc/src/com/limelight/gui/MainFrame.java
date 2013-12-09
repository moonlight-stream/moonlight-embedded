package com.limelight.gui;

import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.UnknownHostException;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JTextField;

import org.xmlpull.v1.XmlPullParserException;

import com.limelight.Limelight;
import com.limelight.binding.PlatformBinding;
import com.limelight.nvstream.NvConnection;
import com.limelight.nvstream.http.NvHTTP;

public class MainFrame {
	private JTextField hostField;
	private JButton pair;
	private JButton stream;
	private JFrame limeFrame;
	
	public JFrame getLimeFrame() {
		return limeFrame;
	}
	
	public void build() {
		limeFrame = new JFrame("Limelight");
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
		
		Box hostBox = Box.createHorizontalBox();
		hostBox.add(Box.createHorizontalStrut(20));
		hostBox.add(hostField);
		hostBox.add(Box.createHorizontalStrut(20));
		
		
		Box contentBox = Box.createVerticalBox();
		contentBox.add(Box.createVerticalStrut(20));
		contentBox.add(hostBox);
		contentBox.add(Box.createVerticalStrut(10));
		contentBox.add(streamBox);
		contentBox.add(Box.createVerticalStrut(10));
		contentBox.add(pairBox);
		contentBox.add(Box.createVerticalGlue());
		
		
		centerPane.add(contentBox);
		mainPane.add(centerPane, "Center");
		
		Dimension dim = Toolkit.getDefaultToolkit().getScreenSize();
		limeFrame.setSize(300, 175);
		limeFrame.setLocation(dim.width/2-limeFrame.getSize().width/2, dim.height/2-limeFrame.getSize().height/2);
		limeFrame.setResizable(false);
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
		return new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent arg0) {
				new Thread(new Runnable() {
					@Override
					public void run() {
						String macAddress;
						try {
							macAddress = NvConnection.getMacAddressString();
						} catch (SocketException e) {
							e.printStackTrace();
							return;
						}
						
						if (macAddress == null) {
							System.out.println("Couldn't find a MAC address");
							return;
						}
						
						NvHTTP httpConn;
						String message;
						try {
							httpConn = new NvHTTP(InetAddress.getByName(hostField.getText()),
									macAddress, PlatformBinding.getDeviceName());
							try {
								if (httpConn.getPairState()) {
									message = "Already paired";
								}
								else {
									int session = httpConn.getSessionId();
									if (session == 0) {
										message = "Pairing was declined by the target";
									}
									else {
										message = "Pairing was successful";
									}
								}
							} catch (IOException e) {
								message = e.getMessage();
							} catch (XmlPullParserException e) {
								message = e.getMessage();
							}
						} catch (UnknownHostException e1) {
							message = "Failed to resolve host";
						}
						
						JOptionPane.showMessageDialog(limeFrame, message, "Limelight", JOptionPane.INFORMATION_MESSAGE);
					}
				}).start();
			}
		};
	}
}
