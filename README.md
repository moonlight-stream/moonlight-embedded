#Limelight-pi

Limelight is an open source implementation of NVIDIA's GameStream, as used by the NVIDIA Shield,
but built for Linux/OSX/Windows.

Limelight-pi allows you to stream your full collection of Steam games from
your powerful Windows desktop to your Raspberry Pi.

[Limelight-common](https://github.com/limelight-stream/limelight-common) is the shared codebase between
different implementations

[Limelight-common](https://github.com/irtimmer/limelight-common) is the forked code used by Limelight-pi

[Limelight](https://github.com/cgutman/limelight) also has an Android
implementation.

[Limelight-pc](https://github.com/limelight-stream/limelight-pc) also has an Linux (X)/OSX/Windows
implementation.

##Features

* Streams Steam and all of your games from your PC to your Raspberry Pi.

##Installation

* Download [GeForce Experience](http://www.geforce.com/geforce-experience) and install on your Windows PC
* Download the appropriate jar from the [GitHub releases page](https://github.com/irtimmer/limelight-pi/releases)
* Install libopus0 (Debian/Raspbian) or opus (ArchLinux/Fedora/Pidora)
* [Configure sound](http://elinux.org/R-Pi_Troubleshooting#Sound)

##Requirements

* [GFE compatible](http://shield.nvidia.com/play-pc-games/) computer with GTX 600/700 series GPU (for the PC you're streaming from)
* High-end wireless router (802.11n dual-band recommended) or Wired network

##Usage

* Ensure your machine and Raspberry Pi are on the same network
* Turn on Shield Streaming in the GFE settings
* Start Limelight-pi with -pair option
* Accept the pairing confirmation on your PC
* Start Limelight-pi normally
* Play games!

##Compile

* Install ant
* Install libopus-dev and libasound2-dev (Debian/Raspbian) or opus-devel and alsa-lib-devel (Fedora/Pidora)
* Build using Ant ``ant``

##Contribute

This project is being actively developed at [XDA](http://forum.xda-developers.com/showthread.php?t=2505510)

1. Fork us
2. Write code
3. Send Pull Requests

##Authors

* [Iwan Timmer](https://github.com/irtimmer)
* [Cameron Gutman](https://github.com/cgutman)  
* [Diego Waxemberg](https://github.com/dwaxemberg)  
* [Aaron Neyer](https://github.com/Aaronneyer)  
* [Andrew Hennessy](https://github.com/yetanothername)
