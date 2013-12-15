package com.limelight.input;

import java.awt.AWTException;
import java.awt.Dimension;
import java.awt.Point;
import java.awt.Robot;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;

import javax.swing.SwingUtilities;

import com.limelight.gui.StreamFrame;
import com.limelight.nvstream.NvConnection;
import com.limelight.nvstream.input.MouseButtonPacket;

public class MouseHandler implements MouseListener, MouseMotionListener {
	private NvConnection conn;
	private Robot robot;
	private Dimension size;
	private StreamFrame parent;
	private int lastX = 0;
	private int lastY = 0;
	private boolean captureMouse = true;
	
	private final double mouseThresh = 0.45;
	
	public MouseHandler(NvConnection conn, StreamFrame parent) {
		this.conn = conn;
		this.parent = parent;
		try {
			this.robot = new Robot();
		} catch (AWTException e) {
			e.printStackTrace();
		}
		size = new Dimension();
	}

	public void free() {
		captureMouse = false;
	}

	public void capture() {
		moveMouse((int)parent.getLocationOnScreen().getX() + (size.width/2),
				(int)parent.getLocationOnScreen().getY() + (size.height/2));
		captureMouse = true;
	}

	@Override
	public void mouseClicked(MouseEvent e) {
		if (captureMouse) {
			parent.hideCursor();
		}
	}

	@Override
	public void mouseEntered(MouseEvent e) {
	}

	@Override
	public void mouseExited(MouseEvent e) {
		if (captureMouse) {
			checkBoundaries(e);
		}
	}

	@Override
	public void mousePressed(MouseEvent e) {
		if (captureMouse) {
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
	}

	@Override
	public void mouseReleased(MouseEvent e) {
		if (captureMouse) {
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
	}

	@Override
	public void mouseDragged(MouseEvent e) {
		if (captureMouse) {
			mouseMoved(e);
		}
	}

	@Override
	public void mouseMoved(MouseEvent e) {
		if (captureMouse) {
			Point mouse = e.getLocationOnScreen();
			int x = (int)mouse.getX();
			int y = (int)mouse.getY();
			conn.sendMouseMove((short)(x - lastX), (short)(y - lastY));
			lastX = x;
			lastY = y;
			
			checkBoundaries(e);
		}
	}

	private void checkBoundaries(MouseEvent e) {
		parent.getSize(size);
		
		int leftEdge = (int) parent.getLocationOnScreen().getX();
		int rightEdge = leftEdge + size.width;
		int upperEdge = (int) parent.getLocationOnScreen().getY();
		int lowerEdge = upperEdge + size.height;
		
		Point mouse = e.getLocationOnScreen();
		
		double xThresh = (size.width * mouseThresh);
		double yThresh = (size.height * mouseThresh);
		
		int newX = (int)mouse.getX();
		int newY = (int)mouse.getY();
		boolean shouldMoveMouse = false;
		
		if (mouse.getX() < leftEdge + xThresh || mouse.getX() > rightEdge - xThresh) {
			newX = (leftEdge + rightEdge) / 2;
			shouldMoveMouse = true;
		}
		if (mouse.getY() < upperEdge + yThresh || mouse.getY() > lowerEdge - yThresh) {
			newY = (upperEdge + lowerEdge) / 2;
			shouldMoveMouse = true;
		}
		
		if (shouldMoveMouse) {
			moveMouse(newX, newY);
		}
	}
	
	private void moveMouse(int x, int y) {
		robot.mouseMove(x, y);
		lastX = x;
		lastY = y;
	}

}
