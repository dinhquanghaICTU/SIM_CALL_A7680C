#!/bin/bash
set -e

echo "=== Cài đặt Toolchain cho STM32F103 (ARM GCC + OpenOCD + ST-Link) ==="
sudo apt-get update
sudo apt-get install -y \
    gcc-arm-none-eabi \
    binutils-arm-none-eabi \
    libnewlib-arm-none-eabi \
    gdb-multiarch \
    openocd \
    stlink-tools \
    make

echo "=== Cấu hình quyền USB cho ST-Link (udev rules) ==="
sudo usermod -a -G dialout $USER
sudo usermod -a -G plugdev $USER

echo "=== Hoàn tất cài đặt! ==="
arm-none-eabi-gcc --version
openocd --version || true
