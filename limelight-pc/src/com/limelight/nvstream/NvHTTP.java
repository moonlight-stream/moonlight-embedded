package com.limelight.nvstream;

import java.io.IOException;
import java.io.InputStream;
import java.net.InetAddress;
import java.net.URL;
import java.net.UnknownHostException;
import java.util.LinkedList;
import java.util.Stack;

import javax.xml.stream.XMLInputFactory;
import javax.xml.stream.XMLStreamException;
import javax.xml.stream.XMLStreamReader;

public class NvHTTP {
	private String macAddress;

	public static final int PORT = 47989;
	public String baseUrl;

	public NvHTTP(String host, String macAddress) {
		this.macAddress = macAddress;
		this.baseUrl = "http://" + host + ":" + PORT;
	}

	private String getXmlString(InputStream in, String tagname)
			throws IOException, XMLStreamException {
		XMLInputFactory factory = XMLInputFactory.newFactory();
		XMLStreamReader xReader = factory.createXMLStreamReader(in);
		
		int eventType = xReader.getEventType();
		Stack<String> currentTag = new Stack<String>();

		while (eventType != XMLStreamReader.END_DOCUMENT) {
			switch (eventType) {
			case (XMLStreamReader.START_ELEMENT):
				currentTag.push(xReader.getElementText());
				break;
			case (XMLStreamReader.END_ELEMENT):
				currentTag.pop();
				break;
			case (XMLStreamReader.CHARACTERS):
				if (currentTag.peek().equals(tagname)) {
					return xReader.getElementText();
				}
				break;
			}
			eventType = xReader.next();
		}

		return null;
	}

	private InputStream openHttpConnection(String url) throws IOException {
		return new URL(url).openConnection().getInputStream();
	}

	public String getAppVersion() throws XMLStreamException, IOException {
		InputStream in = openHttpConnection(baseUrl + "/appversion");
		return getXmlString(in, "appversion");
	}

	public boolean getPairState() throws IOException, XMLStreamException {
		InputStream in = openHttpConnection(baseUrl + "/pairstate?mac=" + macAddress);
		String paired = getXmlString(in, "paired");
		return Integer.valueOf(paired) != 0;
	}

	public int getSessionId() throws IOException, XMLStreamException {
		/* Pass the model (minus spaces) as the device name */
		String deviceName = "Unknown";

		try
		{
		    InetAddress addr;
		    addr = InetAddress.getLocalHost();
		    deviceName = addr.getHostName();
		}
		catch (UnknownHostException ex)
		{
		    System.out.println("Hostname can not be resolved");
		}
		InputStream in = openHttpConnection(baseUrl + "/pair?mac=" + macAddress
				+ "&devicename=" + deviceName);
		String sessionId = getXmlString(in, "sessionid");
		return Integer.parseInt(sessionId);
	}

	public int getSteamAppId(int sessionId) throws IOException,
			XMLStreamException {
		LinkedList<NvApp> appList = getAppList(sessionId);
		for (NvApp app : appList) {
			if (app.getAppName().equals("Steam")) {
				return app.getAppId();
			}
		}
		return 0;
	}
	
	public LinkedList<NvApp> getAppList(int sessionId) throws IOException, XMLStreamException {
		InputStream in = openHttpConnection(baseUrl + "/applist?session=" + sessionId);
		XMLInputFactory factory = XMLInputFactory.newFactory();
		XMLStreamReader xReader = factory.createXMLStreamReader(in);

		int eventType = xReader.getEventType();
		LinkedList<NvApp> appList = new LinkedList<NvApp>();
		Stack<String> currentTag = new Stack<String>();

		while (eventType != XMLStreamReader.END_DOCUMENT) {
			switch (eventType) {
			case (XMLStreamReader.START_ELEMENT):
				currentTag.push(xReader.getName().toString());
				if (xReader.getName().toString().equals("App")) {
					appList.addLast(new NvApp());
				}
				break;
			case (XMLStreamReader.END_DOCUMENT):
				currentTag.pop();
				break;
			case (XMLStreamReader.CHARACTERS):
				NvApp app = appList.getLast();
				if (currentTag.peek().equals("AppTitle")) {
					app.setAppName(xReader.getText());
				} else if (currentTag.peek().equals("ID")) {
					app.setAppId(xReader.getText());
				} else if (currentTag.peek().equals("IsRunning")) {
					app.setIsRunning(xReader.getText());
				}
				break;
			}
			eventType = xReader.next();
		}
		return appList;
	}

	// Returns gameSession XML attribute
	public int launchApp(int sessionId, int appId) throws IOException,
			XMLStreamException {
		InputStream in = openHttpConnection(baseUrl + "/launch?session="
			+ sessionId + "&appid=" + appId);
		String gameSession = getXmlString(in, "gamesession");
		return Integer.parseInt(gameSession);
	}
}
