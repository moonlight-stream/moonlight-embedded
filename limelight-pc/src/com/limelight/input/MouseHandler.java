package com.limelight.input;

import java.awt.AWTException;
import java.awt.Dimension;
import java.awt.Robot;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;

import javax.swing.JFrame;
import javax.swing.SwingUtilities;

import com.limelight.nvstream.NvConnection;
import com.limelight.nvstream.input.MouseButtonPacket;

public class MouseHandler implements MouseListener, MouseMotionListener {
	private NvConnection conn;
	private Robot robot;
	private Dimension size;
	private JFrame parent;
	private int lastX = 0;
	private int lastY = 0;


	public MouseHandler(NvConnection conn, JFrame parent) {
		this.conn = conn;
		this.parent = parent;
		try {
			this.robot = new Robot();
		} catch (AWTException e) {
			e.printStackTrace();
		}
		size = new Dimension();

	}

	@Override
	public void mouseClicked(MouseEvent e) {
	}

	@Override
	public void mouseEntered(MouseEvent e) {
	}

	@Override
	public void mouseExited(MouseEvent e) {
		robot.mouseMove(size.width / 2, size.height / 2);
		lastX = size.width / 2;
		lastY = size.height / 2;
	}

	@Override
	public void mousePressed(MouseEvent e) {
		byte mouseButton = 0x0;
		
		if (SwingUtilities.isLeftMouseButton(e)) {
			mouseButton = MouseButtonPacket.BUTTON_1;
		}
		
		if (SwingUtilities.isMiddleMouseButton(e)) {
			mouseButton = MouseButtonPacket.BUTTON_2;
		}
		
		if (SwingUtilities.isRightMouseButton(e)) {
			mouseButton = MouseButtonPacket.BUTTON_3;
		}
		
		if (mouseButton > 0) {
			conn.sendMouseButtonDown(mouseButton);
		}
	}

	@Override
	public void mouseReleased(MouseEvent e) {
		byte mouseButton = 0x0;
		
		if (SwingUtilities.isLeftMouseButton(e)) {
			mouseButton = MouseButtonPacket.BUTTON_1;
		}
		
		if (SwingUtilities.isMiddleMouseButton(e)) {
			mouseButton = MouseButtonPacket.BUTTON_2;
		}
		
		if (SwingUtilities.isRightMouseButton(e)) {
			mouseButton = MouseButtonPacket.BUTTON_3;
		}
		
		if (mouseButton > 0) {	
			conn.sendMouseButtonUp(mouseButton);
		}
	}

	@Override
	public void mouseDragged(MouseEvent e) {
		mouseMoved(e);
	}

	@Override
	public void mouseMoved(MouseEvent e) {
		int x = e.getX();
		int y = e.getY();
		conn.sendMouseMove((short)(x - lastX), (short)(y - lastY));
		lastX = x;
		lastY = y;
		
		parent.getSize(size);

		if (x < size.width / 2 - 500 || x > size.width / 2 + 500) {
			robot.mouseMove(size.width / 2, y);
			lastX = size.width / 2;
		}
		if (y < size.height / 2 - 300 || y > size.height / 2 + 300) {
			robot.mouseMove(x, size.height / 2);
			lastY = size.height / 2;
		}
		
	}
	
}
