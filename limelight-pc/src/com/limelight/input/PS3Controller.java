package com.limelight.input;

import com.limelight.nvstream.NvConnection;
import com.limelight.nvstream.input.ControllerPacket;

import net.java.games.input.Component;
import net.java.games.input.Controller;

public class PS3Controller extends Gamepad {

	public PS3Controller(NvConnection conn, Controller pad) {
		super(conn, pad);
	}

	@Override
	protected void handleAnalog(Component comp, float value) {
		Component.Identifier id = comp.getIdentifier();
		
		if (id == Component.Identifier.Axis.Z) {
			rightStickX = (short)Math.round(value * 0x7FFF);
		} else if (id == Component.Identifier.Axis.RZ) {
			rightStickY = (short)Math.round(-value * 0x7FFF);
		} else if (id == Component.Identifier.Axis.X) {
			leftStickX = (short)Math.round(value * 0x7FFF);
		} else if (id == Component.Identifier.Axis.Y) {
			leftStickY = (short)Math.round(-value * 0x7FFF);
		} 
	}

	@Override
	protected void handleButtons(Component comp, float value) {
		Component.Identifier id = comp.getIdentifier();
		boolean press = value > 0.5F;
		
		if (id == Component.Identifier.Button._7) {
			toggle(ControllerPacket.LEFT_FLAG, press);
		} else if (id == Component.Identifier.Button._5) {
			toggle(ControllerPacket.RIGHT_FLAG, press);
		} else if (id == Component.Identifier.Button._4) {
			toggle(ControllerPacket.UP_FLAG, press);
		} else if (id == Component.Identifier.Button._6) {
			toggle(ControllerPacket.DOWN_FLAG, press);
		} else if (id == Component.Identifier.Button._14) {
			toggle(ControllerPacket.A_FLAG, press);
		} else if (id == Component.Identifier.Button._15) {
			toggle(ControllerPacket.X_FLAG, press);
		} else if (id == Component.Identifier.Button._12) {
			toggle(ControllerPacket.Y_FLAG, press);
		} else if (id == Component.Identifier.Button._13) {
			toggle(ControllerPacket.B_FLAG, press);
		} else if (id == Component.Identifier.Button._0) {
			toggle(ControllerPacket.BACK_FLAG, press);
		} else if (id == Component.Identifier.Button._3) {
			toggle(ControllerPacket.PLAY_FLAG, press);
		} else if (id == Component.Identifier.Button._2) {
			toggle(ControllerPacket.RS_CLK_FLAG, press);
		} else if (id == Component.Identifier.Button._1) {
			toggle(ControllerPacket.LS_CLK_FLAG, press);
		} else if (id == Component.Identifier.Button._10) {
			toggle(ControllerPacket.LB_FLAG, press);
		} else if (id == Component.Identifier.Button._11) {
			toggle(ControllerPacket.RB_FLAG, press);
		} else if (id == Component.Identifier.Button._16) {
			toggle(ControllerPacket.SPECIAL_BUTTON_FLAG, press);
		} else if (id == Component.Identifier.Button._8) {
			leftTrigger = (byte)Math.round((press ? 1 : 0) * 0xFF);
		} else if (id == Component.Identifier.Button._9) {
			rightTrigger = (byte)Math.round((press ? 1 : 0) * 0xFF);
		}
	}
}
