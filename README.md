#Moonlight Embedded

Moonlight is an open source implementation of NVIDIA's GameStream, as used by the NVIDIA Shield,
but built for Linux/OSX/Windows.

Moonlight Embedded allows you to stream your full collection of Steam games from
your powerful Windows desktop to your embedded system, like Raspberry Pi, CuBox-i and Hummingboard.

For a demo see this [video](https://www.youtube.com/watch?v=XRW6O0bSHNw).

##Features

* Streams Steam and all of your games from your PC to your embedded system.
* Use mDNS to scan for compatible GeForce Experience (GFE) machines on the network.
* Qwerty Keyboard, Mouse and Gamepad support
* Support Raspberry Pi and i.MX 6 devices

##Requirements

* [GFE compatible](http://shield.nvidia.com/play-pc-games/) computer with GTX 600/700/900 series GPU (for the PC you're streaming from)
* High-end wireless router (802.11n dual-band recommended) or wired network
* Geforce Experience 2.1.1 or higher

##Quick Start

* Ensure your machine and embedded system are on the same network
* Turn on Shield Streaming in the GFE settings
* Start Moonlight Embedded with pair
* Accept the pairing confirmation on your PC
* Start Moonlight Embedded with stream
* Play games!

##Usage
	Usage: moonlight [options] host

	Actions:

	map Create mapping file for gamepad
	pair Pair device with computer
	stream Stream computer to device
	list List available games and applications
	quit Quit the application or game being streamed
	help Show this help

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
	-mapping <file> Use <file> as gamepad mapping configuration file (use before -input)
	-audio <device> Use <device> as ALSA audio output device (default sysdefault)
	-localaudio Play audio locally

	Use ctrl-c to exit application

##Compile and install

* Download and extract Moonlight Embedded from release or git clone
* Install development dependencies
* Initialize the git submodules ``git submodule update --init``

```
mkdir build
cd build/
cmake ../
make
make install
```
## Dependencies

ArchLinux/Debian (Raspbian)/Debian Development (Raspbian)
* opus/libopus0/libopus-dev
* expat/libexpat1/libexpat1-dev
* openssl/libssl-dev/libssl-dev
* alsa-lib/libasound2/libasound2-dev
* systemd/libudev/libudev-dev
* avahi/libavahi-client3/libavahi-client-dev
* curl/libcurl3/libcurl4-openssl-dev
* libevdev/libevdev2/libevdev-dev

Development only dependencies
* cmake
* C compiler

## See also

[Moonlight-common-c](https://github.com/moonlight-stream/moonlight-common-c) is the shared codebase between
different C implementations of Moonlight

[Moonlight-common-c](https://github.com/irtimmer/moonlight-common-c) is the used fork used by Moonlight Embedded

[Moonlight-common](https://github.com/moonlight-stream/moonlight-common) is the shared codebase between
different Java implementations of Moonlight

[Moonlight](https://github.com/moonlight-stream/moonlight-android) also has an Android
implementation.

[Moonlight-pc](https://github.com/moonlight-stream/moonlight-pc) also has an Linux/OSX/Windows
implementation.

[Moonlight iOS](https://github.com/moonlight-stream/moonlight-ios) also has an iOS
implementation.

[Moonlight Windows](https://github.com/moonlight-stream/moonlight-windows) also has an Windows and Windows Phone
implementation.

## Discussion

[XDA](http://forum.xda-developers.com/showthread.php?t=2505510)  
[Raspberry Pi Forum](http://www.raspberrypi.org/forums/viewtopic.php?f=78&t=65878)  
[SolidRun Community](http://www.solid-run.com/community/viewtopic.php?f=13&t=1489&p=11173)  

##Contribute

1. Fork us
2. Write code
3. Send Pull Requests
