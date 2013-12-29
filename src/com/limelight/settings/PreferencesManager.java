package com.limelight.settings;

import java.io.File;
import java.io.Serializable;

/**
 * Manages user preferences
 * @author Diego Waxemberg
 */
public abstract class PreferencesManager {
	private static Preferences cachedPreferences = null;
	
	/**
	 * Writes the specified preferences to the preferences file and updates the cached preferences.
	 * @param prefs the preferences to be written out
	 */
	public static void writePreferences(Preferences prefs) {
		System.out.println("Writing Preferences");
		File prefFile = SettingsManager.getInstance().getSettingsFile();
		
		SettingsManager.writeSettings(prefFile, prefs);
		cachedPreferences = prefs;
	}
	
	/**
	 * Reads the user preferences from the preferences file and caches them
	 * @return the user preferences
	 */
	public static Preferences getPreferences() {
		if (cachedPreferences == null) {
			System.out.println("Reading Preferences");
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
	
	/**
	 * Represents a user's preferences
	 * @author Diego Waxemberg
	 */
	public static class Preferences implements Serializable {
		private static final long serialVersionUID = -5575445156215348048L;

		/**
		 * The possible resolutions for the stream
		 */
		public enum Resolution { RES_720_30("1280x720 (30Hz)"), RES_720_60("1280x720 (60Hz)"), 
			RES_1080_30("1920x1080 (30Hz)"), RES_1080_60("1920x1080 (60Hz)");
			public String name;
			
			/*
			 * Creates a new resolution with the specified name
			 */
			private Resolution(String name) {
				this.name = name;
			}
			
			/**
			 * Gets the specified name for this resolution
			 * @return the specified name of this resolution
			 */
			@Override
			public String toString() {
				return name;
			}
		};
		
		private Resolution res;
		private boolean fullscreen;
		private String host;
		
		/**
		 * constructs default preferences: 720p 30Hz fullscreen
		 */
		public Preferences() {
			this(Resolution.RES_720_30, true);
		}
		
		/**
		 * Constructs a preference with the specified values
		 * @param res the <code>Resolution</code> to use
		 * @param fullscreen whether to start the stream in fullscreen
		 */
		public Preferences(Resolution res, boolean fullscreen) {
			this.res = res;
			this.fullscreen = fullscreen;
			this.host = "GeForce PC host";
		}
		
		/**
		 * The saved host in this preference
		 * @return the last used host
		 */
		public String getHost() {
			return host;
		}
		
		/**
		 * Sets the host for this preference
		 * @param host the host to save
		 */
		public void setHost(String host) {
			this.host = host;
		}
		
		/**
		 * Gets the resolution in this preference
		 * @return the stored resolution
		 */
		public Resolution getResolution() {
			return res;
		}
		
		/**
		 * Gets whether to use fullscreen
		 * @return the stored fullscreen mode
		 */
		public boolean getFullscreen() {
			return fullscreen;
		}
		
		/**
		 * Sets the resolution in this preference
		 * @param res the resolution to save
		 */
		public void setResolution(Resolution res) {
			this.res = res;
		}
		
		/**
		 * Sets the fullscreen mode of this preference
		 * @param fullscreen whether to use fullscreen
		 */
		public void setFullscreen(boolean fullscreen) {
			this.fullscreen = fullscreen;
		}
	}
}
