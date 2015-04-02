#Limelight Embedded

Limelight is an open source implementation of NVIDIA's GameStream, as used by the NVIDIA Shield,
but built for Linux/OSX/Windows.

Limelight Embedded allows you to stream your full collection of Steam games from
your powerful Windows desktop to your embedded system, like Raspberry Pi, CuBox-i and Hummingboard.

For a demo see this [video](https://www.youtube.com/watch?v=XRW6O0bSHNw). Instructions on how to set things up on OpenELEC can be found [here.](https://github.com/HazCod/LimeRPi2-kodi/blob/master/README.md)

[Limelight-common](https://github.com/limelight-stream/limelight-common) is the shared codebase between
different implementations of Limelight

[Limelight](https://github.com/limelight-stream/limelight-android) also has an Android
implementation.

[Limelight-pc](https://github.com/limelight-stream/limelight-pc) also has an Linux/OSX/Windows
implementation.

[Limelight iOS](https://github.com/limelight-stream/limelight-ios) also has an iOS
implementation.

[Limelight Windows](https://github.com/limelight-stream/limelight-windows) also has an Windows and Windows Phone
implementation.

##Features

* Streams Steam and all of your games from your PC to your embedded system.

##Installation

* Download [GeForce Experience](http://www.geforce.com/geforce-experience) and install on your Windows PC
* Install oracle-java8-jdk (Raspbian) or download and install [Oracle Java SE (ARM v6 Hard Float)](http://www.oracle.com/technetwork/java/javase/downloads/jdk8-arm-downloads-2187472.html)
* Download the compiled jar from the [GitHub releases page](https://github.com/irtimmer/limelight-pi/releases)
* Install libopus0 (Debian/Raspbian) or opus (Arch Linux/Fedora/Pidora)
* Install v4l-utils, firmware-imx and imx-vpu (Arch Linux) for CuBox-i or Hummingboard
* [Configure sound](http://elinux.org/R-Pi_Troubleshooting#Sound)

##Requirements

* [GFE compatible](http://shield.nvidia.com/play-pc-games/) computer with GTX 600/700/900 series GPU (for the PC you're streaming from)
* High-end wireless router (802.11n dual-band recommended) or wired network
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
	-height <height> Vertical resolution (default 720)
	-30fps Use 30fps
	-60fps Use 60fps (default)
	-bitrate <bitrate> Specify the bitrate in Kbps
	-packetsize <size> Specify the maximum packetsize in bytes
	-app <app> Name of app to stream
	-nosops Don't allow GFE to modify game settings
	-input <device> Use <device> as input. Can be used multiple times
	(default uses all devices in /dev/input)
	-mapping <file> Use <file> as gamepad mapping configuration file
	-audio <device> Use <device> as ALSA audio output device (default sysdefault)
	-localaudio Play audio locally

	Use ctrl-c to exit application

##Fixed point Opus on Debian
Debian/Raspbian currently doesn't provide a fixed point Opus build.
This results in very high cpu usage for audio decompression, especially on the Raspberry Pi.
Limelight Embedded can load another Opus build for you from the current working directory.
To compile Opus with fixed point support you have to execute the following commands:

	wget http://downloads.xiph.org/releases/opus/opus-1.1.tar.gz
	tar xf opus-1.1.tar.gz
	cd opus-1.1
	./configure --enable-fixed-point
	make

Then copy libopus.so from opus-1.1/libs to the directory from which you start Limelight Embedded.

##Compile

* Install ant (Debian/Raspbian/Fedora/Pidora) or apache-ant (Arch Linux)
* Install audio libraries libopus-dev and libasound2-dev (Debian/Raspbian) or opus-devel and alsa-lib-devel (Fedora/Pidora) or opus and alsa-lib (Arch Linux)
* Install for Raspberry Pi development libraries libraspberrypi-dev (Debian/Raspbian) or raspberrypi-vc-libs-devel (Fedora/Pidora) or raspberrypi-firmware-tools (Arch Linux)
* Install for CuBox-i development libraries linux-headers-imx6-cubox-dt (Arch Linux)
* Set JAVA_HOME to your JDK installation directory for example ``export JAVA_HOME=/usr/lib/jvm/jdk-7-oracle-armhf``
* Initialize the git submodules ``git submodule update --init``
* Build using Ant ``ant``

## Discussion

[XDA](http://forum.xda-developers.com/showthread.php?t=2505510)  
[Raspberry Pi Forum](http://www.raspberrypi.org/forums/viewtopic.php?f=78&t=65878)  
[SolidRun Community](http://www.solid-run.com/community/viewtopic.php?f=13&t=1489&p=11173)  

##Contribute

1. Fork us
2. Write code
3. Send Pull Requests
