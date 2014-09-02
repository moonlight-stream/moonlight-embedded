#Limelight Embedded

Limelight is an open source implementation of NVIDIA's GameStream, as used by the NVIDIA Shield,
but built for Linux/OSX/Windows.

Limelight Embedded allows you to stream your full collection of Steam games from
your powerful Windows desktop to your embedded system, like Raspberry Pi, CuBox-i and Hummingboard.

For a demo see this [video](https://www.youtube.com/watch?v=XRW6O0bSHNw).

[Limelight-common](https://github.com/limelight-stream/limelight-common) is the shared codebase between
different implementations of Limelight

[Limelight](https://github.com/cgutman/limelight) also has an Android
implementation.

[Limelight-pc](https://github.com/limelight-stream/limelight-pc) also has an Linux (X)/OSX/Windows
implementation.

##Features

* Streams Steam and all of your games from your PC to your embedded system.

##Installation

* Download [GeForce Experience](http://www.geforce.com/geforce-experience) and install on your Windows PC
* Install oracle-java8-jdk (Raspbian) or download and install [Oracle Java SE (ARM v6 Hard Float)](http://www.oracle.com/technetwork/java/javase/downloads/jdk8-arm-downloads-2187472.html)
* Download the compiled jar from the [GitHub releases page](https://github.com/irtimmer/limelight-pi/releases)
* Install libopus0 (Debian/Raspbian) or opus (ArchLinux/Fedora/Pidora)
* [Configure sound](http://elinux.org/R-Pi_Troubleshooting#Sound)

##Requirements

* [GFE compatible](http://shield.nvidia.com/play-pc-games/) computer with GTX 600/700 series GPU (for the PC you're streaming from)
* High-end wireless router (802.11n dual-band recommended) or Wired network
* Geforce Experience 2.1.1 or higher

##Quick Start

* Ensure your machine and embedded system are on the same network
* Turn on Shield Streaming in the GFE settings
* Start Limelight Embedded with pair
* Accept the pairing confirmation on your PC
* Start Limelight Embedded with stream
* Play games!

##Usage
	Usage: java -jar limelight.jar [options] host

	Actions:

	map Create mapping file for gamepad
	pair Pair device with computer
	stream Stream computer to device
	discover List available computers
	list List available games and applications
	help Show this help

	Mapping options:

	-input <device> Use <device> as input

	Streaming options:

	-720 Use 1280x720 resolution (default)
	-1080 Use 1920x1080 resolution
	-width <width> Horizontal resolution (default 1280)
	-height <height>Vertical resolution (default 720)
	-30fps Use 30fps
	-60fps Use 60fps (default)
	-bitrate <bitrate>Specify the bitrate in Kbps
	-app <app> Name of app to stream
	-nosops Don't allow GFE to modify game settings
	-input <device> Use <device> as input. Can be used multiple times
	(default uses all devices in /dev/input)
	-mapping <file> Use <file> as gamepad mapping configuration file
	-audio <device> Use <device> as ALSA audio output device (default sysdefault)

	Use ctrl-c to exit application

##Compile

* Install ant (Debian/Raspbian/Fedora/Pidora) or apache-ant (Archlinux)
* Install audio libraries libopus-dev and libasound2-dev (Debian/Raspbian) or opus-devel and alsa-lib-devel (Fedora/Pidora) or opus and alsa-lib (Archlinux)
* Install Raspberry Pi development libraries libraspberrypi-dev (Debian/Raspbian) or raspberrypi-vc-libs-devel (Fedora/Pidora) or raspberrypi-firmware-tools (Archlinux)
* Set JAVA_HOME to your JDK installation directory for example ``export JAVA_HOME=/usr/lib/jvm/jdk-7-oracle-armhf``
* Build using Ant ``ant``

## Discussion

[XDA](http://forum.xda-developers.com/showthread.php?t=2505510)
[Raspberry Pi Forum](http://www.raspberrypi.org/forums/viewtopic.php?f=78&t=65878)
[SolidRun Community](http://www.solid-run.com/community/viewtopic.php?f=13&t=1489&p=11173)

##Contribute

1. Fork us
2. Write code
3. Send Pull Requests
