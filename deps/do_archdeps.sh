#!/bin/bash
# installs dependencies on arch linux

sudo pacman -Sy
[ $? -ne 0 ] && exit $?
sudo pacman -S archlinux-keyring
[ $? -ne 0 ] && exit $?
sudo pacman -S glfw-x11 openal premake freetype2 git gcc mesa lua lua52
[ $? -ne 0 ] && exit $?
