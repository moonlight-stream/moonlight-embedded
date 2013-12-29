package com.limelight.settings;

import java.io.File;

import com.limelight.input.gamepad.GamepadMapping;

/**
 * Manages the gamepad settings
 * @author Diego Waxemberg
 */
public abstract class GamepadSettingsManager {
	private static GamepadMapping cachedSettings;

	/**
	 * Reads the gamepad settings from the gamepad file and caches the configuration
	 * @return the gamepad settings
	 */
	public static GamepadMapping getSettings() {
		if (cachedSettings == null) {
			System.out.println("Reading Gamepad Settings");
			File gamepadFile = SettingsManager.getInstance().getGamepadFile();
			GamepadMapping savedMapping = (GamepadMapping)SettingsManager.readSettings(gamepadFile);
			cachedSettings = savedMapping;
		}
		if (cachedSettings == null) {
			System.out.println("Unable to get gamepad settings. Using an empty mapping instead.");
			cachedSettings = new GamepadMapping();
			writeSettings(cachedSettings);
		}
		return cachedSettings;
	}

	/**
	 * Writes the specified mapping to the gamepad file and updates the cached settings
	 * @param settings the new gamepad mapping to be written out
	 */
	public static void writeSettings(GamepadMapping settings) {
		cachedSettings = settings;
		System.out.println("Writing Gamepad Settings");
		
		File gamepadFile = SettingsManager.getInstance().getGamepadFile();
		
		SettingsManager.writeSettings(gamepadFile, settings);
	}

}
