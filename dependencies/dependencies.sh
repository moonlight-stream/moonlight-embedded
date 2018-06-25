#!/bin/sh

if [[ -n $(command -v pacman) ]]; then
    PACMAN="pacman"
    MAKEPKG="makepkg"
    TOUCH="touch"
elif [[ -n $(command -v dkp-pacman) ]]; then 
    PACMAN="sudo dkp-pacman"
    MAKEPKG="dkp-makepkg"
    TOUCH="sudo touch"
else 
    echo "ERROR: Could not find pacman (or dkp-pacman); is $DEVKITPRO set and on the path?"
    exit 1
fi

echo "Adding missing <sys/termios.h> to devkitA64"
$(TOUCH) $DEVKITPRO/devkitA64/aarch64-none-elf/include/sys/termios.h

echo "Installing necessary dependencies for moonlight-switch"
$PACMAN -S --needed --noconfirm \
    switch-pkg-config devkitpro-pkgbuild-helpers \
    switch-sdl2 switch-sdl2_gfx switch-sdl2_ttf switch-sdl2_image \
    switch-bzip2 switch-zlib

echo "Building and installing the bundled dependencies"
(cd switch-libexpat; $MAKEPKG --clean --needed --noconfirm; $PACMAN -U --needed --noconfirm switch-libexpat*)
(cd switch-libavcodec; $MAKEPKG --clean --needed --noconfirm; $PACMAN -U --needed --noconfirm switch-libavcodec*)
(cd switch-libopus; $MAKEPKG --clean --needed --noconfirm; $PACMAN -U --needed --noconfirm switch-libopus*)
(cd switch-openssl; $MAKEPKG --clean --needed --noconfirm; $PACMAN -U --needed --noconfirm switch-openssl*)
