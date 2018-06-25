# Moonlight Switch - Development Guide

## Source

Grab the source of Moonlight Switch via Git (and ensure you are recursively cloning the submodule dependencies):

```
git clone --recursive https://github.com/kbhomes/moonlight-switch.git
```

## Dependencies

Ensure that you have followed instructions to [install devKitPro and libnx](http://switchbrew.org/index.php?title=Setting_up_Development_Environment) on your platform.

Dependencies are collected, built, and installed by [`dependencies/dependencies.sh`](https://github.com/kbhomes/moonlight-switch/blob/master/dependencies/dependencies.sh):

```
cd moonlight-switch/dependencies/
./dependencies.sh
```

On some platforms, you may need root privileges, in which case the script will use `sudo` to ask for your password. This dependency preparation script does the following:
- **Adds a missing file `sys/termios.h` to devkitA64**:  
The existing `termios.h` header in devkitA64 includes `sys/termios.h`, and since OpenSSL includes `termios.h`, building that dependency will fail despite the functionality of `termios.h` not even being used. Adding in an empty version of this file is the simplest solution.
- **Applies a patch to `netdb.h` of libnx**:  
The `netdb.h` file in libnx references the `socklen_t` structure but doesn't include `sys/socket.h` where it's defined, causing compilation errors when this file is included. A small patch is applied that adds `#include <sys/socket.h>` to this file.
- **Uses devKitPro's pacman to install existing dependences**:  
Installs dependencies for Moonlight Switch that already exist in the official devKitPro Switch package repository:
    - `switch-pkg-config`
    - `devkitpro-pkgbuild-helpers`
    - `switch-sdl2`, `switch-sdl2_gfx`, `switch-sdl2_ttf`, `switch-sdl2_image`
    - `switch-bzip2`, `switch-zlib`
- **Builds and installs dependencies not yet in an official package repository**:  
These dependencies are required by Moonlight Switch, but are not yet published in any official package repositories:
    - `switch-libavcodec`: used for H.264 video software decoding
    - `switch-libopus`: used for Opus audio decoding
    - `switch-libexpat`: used for XML parsing
    - `switch-openssl`: used for performing HTTPS requests

## Build

Build Moonlight Switch by running `make` in the root directory. In the end, this will create `dist/moonlight-switch.nro`, which is the application.

## Deploy

- Edit `dist/moonlight.ini` to change the IP address to that of your streaming PC. 
- On your SD card, create the folder `/switch/moonlight-switch/` and place `moonlight.ini` in that directory.
- Send `moonlight-switch.nro` to your device by either placing it in `/switch/moonlight-switch/` on your SD card, or use `nxlink`. To see stdout and stderr, use the `-s` server option: `nxlink -s moonlight-switch.nro`

## Troubleshooting

- During dependency installation, you may run into a permission error when building OpenSSL attempting to write inside of `$DEVKITPRO/portlibs/switch/man`. Short of modifying the OpenSSL Makefile more, the solution I used to handle this was to change the owner of the `portlibs` folder to myself: `sudo chown -R $DEVKITPRO/portlibs <myuser>`