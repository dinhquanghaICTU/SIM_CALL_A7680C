# Dự án STM32F103C8T6 - Standard Peripheral Library (StdPeriph)

Dự án mẫu chuẩn cấu trúc sử dụng thư viện **STM32F10x Standard Peripheral Library (StdPeriph_Driver)** và **CMSIS Core / Device**, biên dịch bằng GCC toolchain (`arm-none-eabi-gcc`), hỗ trợ nạp và gỡ lỗi qua ST-Link (OpenOCD / st-flash).

---

## 1. Cấu Trúc Thư Mục

```text
Project_cuong/
├── Libraries/
│   ├── CMSIS/                           # CMSIS Core & Device headers cho STM32F10x
│   └── STM32F10x_StdPeriph_Driver/     # Thư viện Standard Peripheral (inc & src)
│       ├── inc/                         # stm32f10x_gpio.h, stm32f10x_rcc.h, ...
│       └── src/                         # stm32f10x_gpio.c, stm32f10x_rcc.c, ...
├── Startup/
│   ├── startup_stm32f10x_md.s          # Startup code cho dòng Medium-Density (F103C8T6)
│   └── system_stm32f10x.c              # Khởi tạo Clock (HSE 72MHz)
├── User/
│   ├── main.c                          # Code chính (Blink PC13, SysTick delay)
│   ├── stm32f10x_it.c                  # Interrupt Service Routines
│   ├── stm32f10x_it.h                  # Header ngắt
│   └── stm32f10x_conf.h                # Cấu hình bật/tắt các module StdPeriph
├── .vscode/
│   ├── c_cpp_properties.json           # Cấu hình gợi ý code & IntelliSense
│   ├── tasks.json                      # Các tác vụ Build, Clean, Flash trên VS Code
│   └── launch.json                     # Cấu hình Debug Cortex-Debug với OpenOCD
├── stm32_flash.ld                      # Linker script cho STM32F103C8 (Flash 64K, RAM 20K)
├── Makefile                            # Makefile build tự động
├── install_tools.sh                    # Script cài đặt toolchain trên Ubuntu
└── README.md
```

---

## 2. Cài Đặt Toolchain & Công Cụ (Ubuntu)

Chạy script cài đặt tự động:
```bash
./install_tools.sh
```

Hoặc chạy thủ công lệnh sau trong terminal:
```bash
sudo apt update
sudo apt install -y gcc-arm-none-eabi binutils-arm-none-eabi libnewlib-arm-none-eabi openocd stlink-tools make
```

---

## 3. Quản Lý Tự Động Bằng Script `run.sh`

Dự án có sẵn script [`run.sh`](file:///home/quanghaictu/Project_cuong/run.sh) tích hợp đầy đủ các chế độ:

- **Mở Menu tương tác trực quan:**
  ```bash
  ./run.sh
  ```
  *(Hiển thị menu chọn nhanh: Build, Rebuild, Flash, Erase, Mở Minicom, hoặc Build+Flash+Minicom).*

- **Chạy trực tiếp qua lệnh CLI:**
  ```bash
  # 1. Xóa sạch chip Flash (Mass Erase):
  ./run.sh erase

  # 2. Biên dịch firmware:
  ./run.sh build

  # 3. Nạp firmware xuống chip:
  ./run.sh flash

  # 4. Mở Minicom Serial Monitor (tự nhận diện cổng USB, baud 115200):
  ./run.sh minicom
  # Hoặc chỉ định cổng & baudrate khác:
  ./run.sh minicom /dev/ttyUSB0 9600

  # 5. Kết hợp: Build + Flash + Mở Minicom luôn:
  ./run.sh all
  ```

> [!TIP]
> **Mẹo thoát khỏi Minicom:** Nhấn tổ hợp phím **Ctrl + A**, thả tay ra rồi nhấn phím **X**, chọn **Yes** để thoát.

---

## 4. Biên Dịch & Nạp Thủ Công Bằng Makefile

- **Biên dịch:** `make -j$(nproc)`
- **Xóa Flash:** `make erase`
- **Nạp code:** `make flash` (OpenOCD) hoặc `make flash_stlink` (st-flash)
- **Dọn dẹp build:** `make clean`

---

## 5. Tích Hợp Trên VS Code / Antigravity IDE

- **Phím tắt Build:** Nhấn `Ctrl + Shift + B` -> Chọn **Build Project**.
- **Chạy Flash:** Nhấn `Ctrl + Shift + P` -> Gõ `Tasks: Run Task` -> Chọn **Flash Firmware (OpenOCD)**.
- **Debug:** Cài extension **Cortex-Debug**, sau đó nhấn `F5` để debug từng dòng mã nguồn với Breakpoint.
