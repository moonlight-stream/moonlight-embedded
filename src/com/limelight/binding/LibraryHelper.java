package com.limelight.binding;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.HashSet;

public class LibraryHelper {
	private static final HashSet<String> avcDependencies = new HashSet<String>();
	private static final boolean needsDependencyExtraction;
	private static final String libraryExtractionFolder;

	private static boolean librariesExtracted = false;
	
	static {
		needsDependencyExtraction = System.getProperty("os.name", "").contains("Windows");
		libraryExtractionFolder = System.getProperty("java.io.tmpdir", ".");
		
		// FFMPEG libraries
		avcDependencies.add("avutil-52");
		avcDependencies.add("swresample-0");
		avcDependencies.add("swscale-2");
		avcDependencies.add("avcodec-55");
		avcDependencies.add("avformat-55");
		avcDependencies.add("avfilter-3");
		
		// The AVC JNI library itself
		avcDependencies.add("nv_avc_dec");
		
		// Additional Windows dependencies
		if (System.getProperty("os.name").contains("Windows")) {
			avcDependencies.add("postproc-52");
			avcDependencies.add("pthreadVC2");
		}
	}
	
	public static void loadNativeLibrary(String libraryName) {
		if (librariesExtracted && avcDependencies.contains(libraryName)) {
			System.load(libraryExtractionFolder + File.separatorChar + System.mapLibraryName(libraryName));
		}
		else {
			System.loadLibrary(libraryName);
		}
	}

	public static void prepareNativeLibraries() {
		if (!needsDependencyExtraction) {
			return;
		}
		
		try {
			for (String dependency : avcDependencies) {
				extractNativeLibrary(dependency);
			}
		} catch (IOException e) {
			// This is expected if this code is not running from a JAR
			return;
		}
		
		librariesExtracted = true;
	}
	
	private static void extractNativeLibrary(String libraryName) throws IOException {
		// convert general library name to platform-specific name
		libraryName = System.mapLibraryName(libraryName);
		
		InputStream resource = new Object().getClass().getResourceAsStream("/binlib/"+libraryName);
		if (resource == null) {
			throw new FileNotFoundException("Unable to find native library in JAR: "+libraryName);
		}
		File destination = new File(libraryExtractionFolder+File.separatorChar+libraryName);
		
		// this will only delete it if it exists, and then create a new file
		destination.delete();
		destination.createNewFile();
		
		// schedule the temporary file to be deleted when the program exits
		destination.deleteOnExit();
		
		//this is the janky java 6 way to copy a file
		FileOutputStream fos = null;
		try {
			fos = new FileOutputStream(destination);
			int read;
			byte[] readBuffer = new byte[16384];
			while ((read = resource.read(readBuffer)) != -1) {
				fos.write(readBuffer, 0, read);
			}
		} finally {
			if (fos != null) {
				fos.close();
			}
		}
	}
	
	public static boolean isRunningFromJar() {
		String classPath = LibraryHelper.class.getResource("LibraryHelper.class").toString();
		return classPath.startsWith("jar:");
	}
}
