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

/**
 * Handles mouse input and sends them via the connection to the host
 * @author Diego Waxemberg
 */
public class MouseHandler implements MouseListener, MouseMotionListener {
	private NvConnection conn;
	private Robot robot;
	private Dimension size;
	private StreamFrame parent;
	private int lastX = 0;
	private int lastY = 0;
	private boolean captureMouse = true;
	
	private final double mouseThresh = 0.45;
	
	/**
	 * Constructs a new handler for the specified connection and belonging to the specified frame
	 * @param conn the connection to which mouse events will be sent
	 * @param parent the frame that owns this handler
	 */
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

	/**
	 * Frees the mouse, that is stops capturing events and allows it to move freely
	 */
	public void free() {
		captureMouse = false;
	}

	/**
	 * Starts capturing mouse events and limits its motion
	 */
	public void capture() {
		moveMouse((int)parent.getLocationOnScreen().getX() + (size.width/2),
				(int)parent.getLocationOnScreen().getY() + (size.height/2));
		captureMouse = true;
	}

	/**
	 * Only used to hide the cursor when the user clicks back into the frame.
	 * <br>The event is not sent to the host
	 * @param e click event used to know that the cursor should now be hidden
	 */
	@Override
	public void mouseClicked(MouseEvent e) {
		if (captureMouse) {
			parent.hideCursor();
		}
	}

	/**
	 * Unimplemented
	 * @param e Unused
	 */
	@Override
	public void mouseEntered(MouseEvent e) {
	}

	/**
	 * Invoked when the mouse leaves the frame.
	 * <br>If this happens when we are capturing the mouse, the mouse is moved back to the center of the frame.
	 * @param e the event created by the mouse leaving the frame
	 */
	@Override
	public void mouseExited(MouseEvent e) {
		if (captureMouse) {
			checkBoundaries(e);
		}
	}

	/**
	 * Invoked when a mouse button is pressed.
	 * <br>The button pressed is sent to the host if we are capturing the mouse.
	 * @param e event containing the mouse button that was pressed
	 */
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

	/**
	 * Invoked when a mouse button is released.
	 * <br>The button released is sent to the host if we are capturing the mouse.
	 * @param e event containing the mouse button that was released
	 */
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

	/**
	 * Invoked when the mouse is dragged, that is moved while a button is held down.
	 * <br>This method simply calls the <code>mouseMoved()</code> method because GFE handles movements all the same
	 * when a button is held down or not.
	 */
	@Override
	public void mouseDragged(MouseEvent e) {
		if (captureMouse) {
			mouseMoved(e);
		}
	}

	/**
	 * Invoked when the mouse is moved.
	 * <br>The change in position is calculated and sent to the host.
	 * <br>If the mouse moves outside a certain boundary, the mouse is moved back to the center- this gives the user
	 * the illusion that they are controlling the mouse they see rather than their own.
	 * @param e the mouse move event containing the new location of the mouse
	 */
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

	/*
	 * Checks if the mouse has moved outside the boundaries.
	 * If so, the mouse is moved back to the center.
	 */
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
	
	/*
	 * Moves the mouse to the specified coordinates on-screen
	 */
	private void moveMouse(int x, int y) {
		robot.mouseMove(x, y);
		lastX = x;
		lastY = y;
	}

}
