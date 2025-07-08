#!/bin/bash

# Blue Enviroment Installer
# Author: Michal332111
# From Organisation: VoidArc Studio
# Description: Installs all dependencies and compiles Blue Enviroment automatically

set -e

echo "======================================"
echo "  Blue Enviroment Installer"
echo "======================================"

# Ensure script has execution permission
if [ ! -x "$0" ]; then
  chmod a+x "$0"
fi

# Update system
echo "[*] Updating system..."
sudo apt update
sudo apt upgrade -y

# Install Wayfire dependencies
echo "[*] Installing Wayfire dependencies..."
sudo apt install -y \
  build-essential meson ninja-build pkg-config \
  libpango1.0-dev libcairo2-dev libfreetype6-dev \
  libglm-dev libdrm-dev libevdev-dev libgl1-mesa-dev \
  libinput-dev libjpeg-dev libpng-dev libxkbcommon-dev \
  libxml2-dev libpixman-1-dev wayland-protocols \
  libwayland-dev

# Install wf-config
echo "[*] Installing wf-config dependencies..."
sudo apt install -y cmake

# Install wlroots dependencies (DRM Backend)
echo "[*] Installing wlroots DRM backend dependencies..."
sudo apt install -y libdisplay-info-dev hwdata

# Install GLES2 renderer dependencies
echo "[*] Installing GLES2 renderer dependencies..."
sudo apt install -y libglvnd-dev libegl1-mesa-dev libgbm-dev

# Install Libinput Backend dependencies
echo "[*] Installing Libinput backend dependencies..."
sudo apt install -y libinput-dev

# Install Session Provider dependencies
echo "[*] Installing session provider dependencies..."
sudo apt install -y libudev-dev seatd

# Install XWayland Support dependencies (optional)
echo "[*] Installing optional XWayland support dependencies..."
sudo apt install -y libxcb1-dev libxcb-composite0-dev libxcb-render0-dev libxcb-xfixes0-dev

# Install X11 Backend dependencies (optional)
echo "[*] Installing optional X11 backend dependencies..."
sudo apt install -y libx11-xcb-dev libxcb-xinput-dev

echo "[*] All dependencies installed successfully."

# Clone Blue Enviroment if not already cloned
if [ ! -d "Blue-Enviroment" ]; then
  echo "[*] Cloning Blue Enviroment repository..."
  git clone https://github.com/VoidArc-Studio/Blue-Enviroment.git
fi

cd Blue-Enviroment

# Ensure install.sh has execution permission
if [ ! -x "./install.sh" ]; then
  chmod a+x ./install.sh
fi

# Run install.sh
echo "[*] Running Blue Enviroment install.sh..."
./install.sh

echo "======================================"
echo " Blue Enviroment installation complete"
echo "======================================"
