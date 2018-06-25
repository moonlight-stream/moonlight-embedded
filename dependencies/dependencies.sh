#!/bin/sh

echo "Adding missing <sys/termios.h> to devkitA64"
sudo touch $DEVKITPRO/devkitA64/aarch64-none-elf/include/sys/termios.h

echo "Installing necessary dependencies for moonlight-switch"
sudo dkp-pacman -S --needed --noconfirm \
    switch-pkg-config devkitpro-pkgbuild-helpers \
    switch-sdl2 switch-sdl2_gfx switch-sdl2_ttf switch-sdl2_image \
    switch-bzip2 switch-zlib

echo "Building and installing the bundled dependencies"
(cd switch-libexpat; dkp-makepkg --clean --needed --noconfirm; sudo dkp-pacman -U --needed --noconfirm switch-libexpat*)
(cd switch-libavcodec; dkp-makepkg --clean --needed --noconfirm; sudo dkp-pacman -U --needed --noconfirm switch-libavcodec*)
(cd switch-libopus; dkp-makepkg --clean --needed --noconfirm; sudo dkp-pacman -U --needed --noconfirm switch-libopus*)
(cd switch-openssl; dkp-makepkg --clean --needed --noconfirm; sudo dkp-pacman -U --needed --noconfirm switch-openssl*)
