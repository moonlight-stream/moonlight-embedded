#!/bin/sh

if [[ -n $(command -v pacman) ]]; then
    PACMAN="pacman"
    MAKEPKG="makepkg"
elif [[ -n $(command -v dkp-pacman) ]]; then 
    PACMAN="sudo dkp-pacman"
    MAKEPKG="dkp-makepkg"
else 
    echo "ERROR: Could not find pacman (or dkp-pacman); is $DEVKITPRO set and on the path?"
    exit 1
fi

export MOONLIGHT_DEPDIR=`pwd`
echo "MOONLIGHT_DEPDIR=$MOONLIGHT_DEPDIR"

echo "Installing necessary dependencies for moonlight-switch"
$PACMAN -S --needed --noconfirm \
    switch-pkg-config devkitpro-pkgbuild-helpers \
    switch-sdl2 switch-sdl2_gfx switch-sdl2_ttf switch-sdl2_image \
    switch-libexpat switch-libopus switch-bzip2 switch-zlib

echo "Building and installing the bundled dependencies to $MOONLIGHT_DEPDIR"
(cd switch-libavcodec; $MAKEPKG --needed --noconfirm;)
(cd switch-openssl; $MAKEPKG --needed --noconfirm;)
