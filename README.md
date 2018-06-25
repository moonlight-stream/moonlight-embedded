<p align="center">
  <img src="https://raw.githubusercontent.com/kbhomes/moonlight-switch/master/data/moonlight_switch_logo.png" />
</p>

Moonlight Switch allows you to stream your collection of games on your GeForce-equipped PC directly to your Nintendo Switch. Based on @irtimmer's wonderful [`moonlight-embedded`](https://github.com/irtimmer/moonlight-embedded).

This is currently in an utterly **unusable state**, since Nintendo Switch homebrew do not yet have access to the hardware video decoder (as of this writing). Once that functionality is available, however, Moonlight Switch should be ready to go shortly thereafter.

## Requirements

* Nintendo Switch with homebrew capability
* [GFE compatible](http://shield.nvidia.com/play-pc-games/) computer with GeForce Experience 2.1.1 or higher

## Quick start

* Deply:
    - Edit `moonlight.ini` to change the IP address to that of your streaming PC. 
    - On your SD card, create the folder `/switch/moonlight-switch/` and place `moonlight.ini` in that directory.
    - Send `moonlight-switch.nro` to your device by either placing it in `/switch/moonlight-switch/` on your SD card, or use `nxlink`. To see stdout and stderr, use the `-s` server option: `nxlink -s moonlight-switch.nro`
* Running:
    * Ensure your Nintendo Switch and streaming PC are on the same network
    * Turn on Shield Streaming in the GFE settings
    * Run the Moonlight Switch homebrew application
    * Play games!

## Bugs

Bugs can be reported to the [issue tracker](https://github.com/kbhomes/moonlight-switch/issues).

## Development

See the [development guide](https://github.com/kbhomes/moonlight-switch/blob/master/DEVELOPMENT.md) for information on how to pull the sources, install the dependencies, and build the application.

## See also

- [moonlight-common-c](https://github.com/kbhomes/moonlight-common-c) is the fork of the Moonlight common core used by Moonlight Switch
- [libswitchui](https://github.com/kbhomes/libswitchui) is the UI library I developed to create interfaces that emulate Nintendo Switch official software

## Donations

The largest portion of this software was originally developed by @irtimmer in `moonlight-embedded`, so please send any donations his way! I've included my PayPal as well if you are so inclined! :)

- @irtimmer
    - Bitcoin [1DgheY9CkQhzwgtjaoYpGSudaMzck1swDp](bitcoin:1DgheY9CkQhzwgtjaoYpGSudaMzck1swDp)
    - [PayPal](https://www.paypal.me/itimmer)
    - [Flattr](https://flattr.com/submit/auto?fid=lz111v&url=https%3A%2F%2Fgithub.com%2Firtimmer%2Fmoonlight-embedded)
- @kbhomes
    - [PayPal](https://paypal.me/sajidanwar94)
