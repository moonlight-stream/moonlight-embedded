#Moonlight Embedded

Moonlight is an open source implementation of NVIDIA's GameStream, as used by the NVIDIA Shield,
but built for Linux/OSX/Windows.

Moonlight Embedded allows you to stream your full collection of Steam games from
your powerful Windows desktop to your embedded system, like Raspberry Pi, CuBox-i and Hummingboard.

For a demo see this [video](https://www.youtube.com/watch?v=XRW6O0bSHNw).

[Moonlight-common-c](https://github.com/moonlight-stream/moonlight-common-c) is the shared codebase between
different C implementations of Moonlight

[Moonlight](https://github.com/moonlight-stream/moonlight-android) also has an Android
implementation.

[Moonlight-pc](https://github.com/moonlight-stream/moonlight-pc) also has an Linux/OSX/Windows
implementation.

[Moonlight iOS](https://github.com/moonlight-stream/moonlight-ios) also has an iOS
implementation.

[Moonlight Windows](https://github.com/moonlight-stream/moonlight-windows) also has an Windows and Windows Phone
implementation.

##Features

* Streams Steam and all of your games from your PC to your embedded system.

##Installation

* Download [GeForce Experience](http://www.geforce.com/geforce-experience) and install on your Windows PC
* Download the compiled version from the [GitHub releases page](https://github.com/irtimmer/moonlight-embedded/releases)
* [Configure sound](http://elinux.org/R-Pi_Troubleshooting#Sound)

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
	Usage: moonlight host game

##Compile

* Install cmake
* Install cryptographics libraries libssl-dev (Debian/Raspbian) or openssl-devel (Fedora/Pidora) or openssl (Arch Linux)
* Initialize the git submodules ``git submodule update --init``

```
mkdir build
cd build/
cmake ../
make
```

## Discussion

[XDA](http://forum.xda-developers.com/showthread.php?t=2505510)  
[Raspberry Pi Forum](http://www.raspberrypi.org/forums/viewtopic.php?f=78&t=65878)  
[SolidRun Community](http://www.solid-run.com/community/viewtopic.php?f=13&t=1489&p=11173)  

##Contribute

1. Fork us
2. Write code
3. Send Pull Requests
