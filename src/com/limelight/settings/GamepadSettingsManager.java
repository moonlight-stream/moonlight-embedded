package com.limelight.settings;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;

import com.limelight.input.GamepadMapping;

public class GamepadSettingsManager {
	private static GamepadMapping cachedSettings;


	public static GamepadMapping getSettings() {
		if (cachedSettings == null) {
			System.out.println("Reading Gamepad Settings");
			File gamepadFile = SettingsManager.getInstance().getGamepadFile();
			ObjectInputStream ois = null;

			try {
				ois = new ObjectInputStream(new FileInputStream(gamepadFile));
				GamepadMapping savedSettings = (GamepadMapping)ois.readObject();
				cachedSettings = savedSettings;

			} catch (ClassNotFoundException e) {
				System.out.println("Saved file is not of the correct type. It might have been modified externally.");

			} catch (FileNotFoundException e) {
				System.out.println("Could not find gamepad settings file");
				e.printStackTrace();

			} catch (IOException e) {
				System.out.println("Could not read gamepad settings file");
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
		}
		if (cachedSettings == null) {
			System.out.println("Unable to get gamepad settings. Using an empty mapping instead.");
			cachedSettings = new GamepadMapping();
			writeSettings(cachedSettings);
		}
		return cachedSettings;
	}

	public static void writeSettings(GamepadMapping settings) {
		cachedSettings = settings;
		System.out.println("Writing Gamepad Settings");
		
		File gamepadFile = SettingsManager.getInstance().getGamepadFile();

		ObjectOutputStream ous = null;

		try {
			ous = new ObjectOutputStream(new FileOutputStream(gamepadFile));
			ous.writeObject(settings);

		} catch (FileNotFoundException e) {
			System.out.println("Could not find gamepad settings file");
			e.printStackTrace();

		} catch (IOException e) {
			System.out.println("Could not write gamepad settings file");
			e.printStackTrace();

		} finally {
			if (ous != null) {
				try {
					ous.close();
				} catch (IOException e) {
					System.out.println("Unable to close gamepad settings file");
					e.printStackTrace();
				}
			}
		}
	}

}
