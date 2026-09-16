#!/bin/bash
# ==============================================================================
# Script cấp quyền USB cho ST-Link và USB-UART (CH340/FTDI/CP210x) trên Linux
# ==============================================================================

set -e

echo ">>> Đang thiết lập quyền truy cập USB cho ST-Link và USB-UART..."

cat << 'EOF' | sudo tee /etc/udev/rules.d/99-stlink.rules > /dev/null
# ST-Link V1
ATTRS{idVendor}=="0483", ATTRS{idProduct}=="3744", MODE="0666", GROUP="plugdev", TAG+="uaccess"
# ST-Link V2 (phổ biến nhất)
ATTRS{idVendor}=="0483", ATTRS{idProduct}=="3748", MODE="0666", GROUP="plugdev", TAG+="uaccess"
# ST-Link V2-1
ATTRS{idVendor}=="0483", ATTRS{idProduct}=="374b", MODE="0666", GROUP="plugdev", TAG+="uaccess"
ATTRS{idVendor}=="0483", ATTRS{idProduct}=="3752", MODE="0666", GROUP="plugdev", TAG+="uaccess"
# ST-Link V3
ATTRS{idVendor}=="0483", ATTRS{idProduct}=="374d", MODE="0666", GROUP="plugdev", TAG+="uaccess"
ATTRS{idVendor}=="0483", ATTRS{idProduct}=="374e", MODE="0666", GROUP="plugdev", TAG+="uaccess"
ATTRS{idVendor}=="0483", ATTRS{idProduct}=="374f", MODE="0666", GROUP="plugdev", TAG+="uaccess"
ATTRS{idVendor}=="0483", ATTRS{idProduct}=="3753", MODE="0666", GROUP="plugdev", TAG+="uaccess"
ATTRS{idVendor}=="0483", ATTRS{idProduct}=="3754", MODE="0666", GROUP="plugdev", TAG+="uaccess"
EOF

cat << 'EOF' | sudo tee /etc/udev/rules.d/99-serial.rules > /dev/null
# CH340, FTDI, CP210x, PL2303
KERNEL=="ttyUSB[0-9]*", MODE="0666", GROUP="dialout"
KERNEL=="ttyACM[0-9]*", MODE="0666", GROUP="dialout"
EOF

echo ">>> Thêm user $USER vào nhóm dialout và plugdev..."
sudo usermod -a -G dialout "$USER" || true
sudo usermod -a -G plugdev "$USER" || true

echo ">>> Tải lại cấu hình udev..."
sudo udevadm control --reload-rules
sudo udevadm trigger

echo "=== ĐÃ HOÀN TẤT ==="
echo "Nếu vẫn báo lỗi, vui lòng rút ST-Link ra và cắm lại vào cổng USB."
