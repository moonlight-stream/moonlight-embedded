package com.limelight.settings;

import java.io.File;
import java.io.IOException;

public class SettingsManager {
	
	public static String SETTINGS_DIR = System.getProperty("user.home") + File.separator + "Limelight";
	
	//directory to hold limelight settings
	private File settingsDir;
	
	private File settingsFile;
	private File gamepadFile;
	
	private static SettingsManager manager;
	
	private SettingsManager() {
		settingsFile = new File(SETTINGS_DIR + File.separator + "settings.lime");
		gamepadFile = new File(SETTINGS_DIR + File.separator + "gamepad.lime");
		settingsDir = new File(SETTINGS_DIR);
	}
	
	public static SettingsManager getInstance() {
		if (manager == null) {
			manager = new SettingsManager();
		}
		return manager;
	}
	
	public File getGamepadFile() {
		if (!settingsDir.exists()) {
			settingsFile.mkdirs();
		}
		
		if (!gamepadFile.exists()) {
			try {
				gamepadFile.createNewFile();
			} catch (IOException e) {
				System.out.println("Unable to create gamepad file");
				return null;
			}
		}
		
		return gamepadFile;
	}
	
	public File getSettingsFile() {
		if (!settingsDir.exists()) {
			settingsFile.mkdirs();
		}
		
		if (!settingsFile.exists()) {
			try {
				settingsFile.createNewFile();
			} catch (IOException e) {
				System.out.println("Unable to create setting file");
				return null;
			}
		}
		
		return settingsFile;
	}
	
}
