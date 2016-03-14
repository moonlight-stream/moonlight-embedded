# Moonlight Embedded

Moonlight Embedded is an open source implementation of NVIDIA's GameStream, as used by the NVIDIA Shield, but built for Linux.

Moonlight Embedded allows you to stream your full collection of Steam games from
your powerful Windows desktop to your (embedded) Linux system, like ODROID, Raspberry Pi, CuBox-i and Hummingboard.

## Documentation

More information about installing and runnning Moonlight Embedded is available on the [wiki](https://github.com/irtimmer/moonlight-embedded/wiki).

## Features

* Streams Steam and all of your games from your PC to your embedded system.
* Use mDNS to scan for compatible GeForce Experience (GFE) machines on the network.
* Qwerty Keyboard, Mouse and Gamepad support
* Support H264 hardware video decoding on ODROID, Raspberry Pi and i.MX 6 devices
* Support HEVC hardware video decoding on ODROID C1/C2

## Requirements

* [GFE compatible](http://shield.nvidia.com/play-pc-games/) computer with GTX 600/700/900 series GPU (for the PC you're streaming from)
* High-end wireless router (802.11n dual-band recommended) or wired network
* Geforce Experience 2.1.1 or higher

## Quick Start

* Ensure your GFE server and client are on the same network
* Turn on Shield Streaming in the GFE settings
* Pair Moonlight Embedded with the GFE server
* Accept the pairing confirmation on your PC
* Connect to the GFE Server with Moonlight Embedded
* Play games!

## Bugs

Please check the fora, wiki and old bug reports before submitting a new bug report.

Bugs can be reported to the [issue tracker](https://github.com/irtimmer/moonlight-embedded/issues).

## See also

[Moonlight-common-c](https://github.com/moonlight-stream/moonlight-common-c) is the shared codebase between
different C implementations of Moonlight

[Moonlight-common-c](https://github.com/irtimmer/moonlight-common-c) is the fork used by Moonlight Embedded

## Discussion

[XDA](http://forum.xda-developers.com/showthread.php?t=2505510)  
[Raspberry Pi Forum](http://www.raspberrypi.org/forums/viewtopic.php?f=78&t=65878)  
[SolidRun Community](http://www.solid-run.com/community/viewtopic.php?f=13&t=1489&p=11173)  
[ODROID Forum](http://forum.odroid.com/viewtopic.php?f=91&t=15456) Moonlight Embedded on ODROID  

## Contribute

1. Fork us
2. Write code
3. Send Pull Requests
