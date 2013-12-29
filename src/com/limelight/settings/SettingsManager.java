package com.limelight.settings;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;

/**
 * Manages the settings files
 * @author Diego Waxemberg
 */
public class SettingsManager {
	/**
	 * Directory to which settings will be saved
	 */
	public static String SETTINGS_DIR = System.getProperty("user.home") + File.separator + "Limelight";
	
	//directory to hold limelight settings
	private File settingsDir;
	
	private File settingsFile;
	private File gamepadFile;
	
	private static SettingsManager manager;
	
	/*
	 * Constructs a manager that initializes the settings files
	 */
	private SettingsManager() {
		settingsFile = new File(SETTINGS_DIR + File.separator + "settings.lime");
		gamepadFile = new File(SETTINGS_DIR + File.separator + "gamepad.lime");
		settingsDir = new File(SETTINGS_DIR);
	}
	
	/**
	 * Gets an instance of the manager, this is a singleton
	 * @return the instance of the manager
	 */
	public static SettingsManager getInstance() {
		if (manager == null) {
			manager = new SettingsManager();
		}
		return manager;
	}
	
	/**
	 * Gets the gamepad preference file, if the file does not exist, it is created first
	 * @return the gamepad preference file
	 */
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
	
	/**
	 * Gets the settings file (user preferences), if the file does not exist, it is created first
	 * @return the settings file
	 */
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
	
	/**
	 * Reads the specified file as a settings file and returns the result.
	 * <br>A settings file must be a java serialized object
	 * @param file the file to read in
	 * @return the settings represented in this file
	 */
	public static Object readSettings(File file) {
		ObjectInputStream ois = null;
		Object settings = null;
		try {
			ois = new ObjectInputStream(new FileInputStream(file));
			Object savedSettings = ois.readObject();
			settings = savedSettings;
		} catch (ClassNotFoundException e) {
			System.out.println("Saved file is not of the correct type. It might have been modified externally.");

		} catch (FileNotFoundException e) {
			System.out.println("Could not find " + file.getName() + " settings file");
			e.printStackTrace();

		} catch (IOException e) {
			System.out.println("Could not read " + file.getName() + " settings file");
			e.printStackTrace();

		} finally {
			if (ois != null) {
				try {
					ois.close();

				} catch (IOException e) {
					System.out.println("Could not close gamepad settings file");
					e.printStackTrace();
				}
			}
		}
		return settings;
	}
	
	/**
	 * Writes the specified settings to the desired file
	 * @param file the file to write the settings to 
	 * @param settings the settings to be written out
	 */
	public static void writeSettings(File file, Serializable settings) {
		ObjectOutputStream ous = null;

		try {
			ous = new ObjectOutputStream(new FileOutputStream(file));
			ous.writeObject(settings);

		} catch (FileNotFoundException e) {
			System.out.println("Could not find " + file.getName() + " settings file");
			e.printStackTrace();

		} catch (IOException e) {
			System.out.println("Could not write to " + file.getName() + " settings file");
			e.printStackTrace();

		} finally {
			if (ous != null) {
				try {
					ous.close();
				} catch (IOException e) {
					System.out.println("Unable to close " + file.getName() + " settings file");
					e.printStackTrace();
				}
			}
		}
	}
}
