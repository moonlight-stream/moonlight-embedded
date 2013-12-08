package com.limelight.input;

import java.awt.AWTException;
import java.awt.Dimension;
import java.awt.Robot;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;

import javax.swing.JFrame;

import com.limelight.nvstream.NvConnection;

public class MouseHandler implements MouseListener, MouseMotionListener {
	private NvConnection conn;
	private Robot robot;
	private JFrame parent;
	private Dimension size;
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
		System.out.println("Mouse Exiting!");
		parent.getSize(size);
		robot.mouseMove(size.width / 2, size.height / 2);
		lastX = size.width / 2;
		lastY = size.height / 2;
	}

	@Override
	public void mousePressed(MouseEvent e) {
		conn.sendMouseButtonDown();
		e.consume();
	}

	@Override
	public void mouseReleased(MouseEvent e) {
		conn.sendMouseButtonUp();
		e.consume();
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
		e.consume();
	}

}
