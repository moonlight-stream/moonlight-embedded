package com.limelight.settings;

import java.io.File;
import java.io.Serializable;

public abstract class PreferencesManager {
	private static Preferences cachedPreferences = null;
	
	public static void writePreferences(Preferences prefs) {
		File prefFile = SettingsManager.getInstance().getSettingsFile();
		
		SettingsManager.writeSettings(prefFile, prefs);
	}
	
	public static Preferences getPreferences() {
		if (cachedPreferences == null) {
			File prefFile = SettingsManager.getInstance().getSettingsFile();
			Preferences savedPref = (Preferences)SettingsManager.readSettings(prefFile);
			cachedPreferences = savedPref;
		}
		if (cachedPreferences == null) {
			System.out.println("Unabled to get preferences, using default");
			cachedPreferences = new Preferences();
			writePreferences(cachedPreferences);
		}
		return cachedPreferences;
	}
	
	public static class Preferences implements Serializable {
		private static final long serialVersionUID = -5575445156215348048L;

		public enum Resolution { RES_720, RES_1080 };
		
		private Resolution res;
		private boolean fullscreen;
		
		/**
		 * construcs default preferences: 720p fullscreen
		 */
		public Preferences() {
			this.res = Resolution.RES_720;
			this.fullscreen = true;
		}
		
		public Preferences(Resolution res, boolean fullscreen) {
			this.res = res;
			this.fullscreen = fullscreen;
		}
		
		public Resolution getResolution() {
			return res;
		}
		
		public boolean getFullscreen() {
			return fullscreen;
		}
		
		public void setResolution(Resolution res) {
			this.res = res;
		}
		
		public void setFullscreen(boolean fullscreen) {
			this.fullscreen = fullscreen;
		}
	}
}
