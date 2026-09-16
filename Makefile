##########################################################################################################################
# Makefile for STM32F103C8T6 - Standard Peripheral Library (StdPeriph)
# Hỗ trợ tự động quét tất cả thư mục User (src, inc, hardware, middle, application, third_party)
##########################################################################################################################

TARGET = stm32f103_stdperiph
DEBUG = 1
OPT = -Og

BUILD_DIR = build

# -----------------------------------------------------------------------------------------
# Nguồn mã nguồn C & ASM (Tự động quét toàn bộ thư mục User, Startup và Libraries)
# -----------------------------------------------------------------------------------------
C_SOURCES = \
  $(shell find User -name "*.c" 2>/dev/null) \
  $(shell find Startup -name "*.c" 2>/dev/null) \
  $(filter-out %_fsmc.c %_sdio.c %_dac.c, $(wildcard Libraries/STM32F10x_StdPeriph_Driver/src/*.c))

ASM_SOURCES = \
  Startup/startup_stm32f10x_md.s

# -----------------------------------------------------------------------------------------
# Toolchain GCC ARM
# -----------------------------------------------------------------------------------------
PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

# Kiến trúc vi điều khiển STM32F103 (Cortex-M3)
CPU = -mcpu=cortex-m3
MCU = $(CPU) -mthumb

# C Defines
C_DEFS = \
  -DSTM32F10X_MD \
  -DUSE_STDPERIPH_DRIVER

# -----------------------------------------------------------------------------------------
# C Includes (Tự động quét toàn bộ thư mục con trong User để include)
# -----------------------------------------------------------------------------------------
USER_INC_DIRS = $(shell find User -type d 2>/dev/null)

C_INCLUDES = \
  -IUser \
  $(addprefix -I, $(USER_INC_DIRS)) \
  -IStartup \
  -ILibraries/CMSIS/Include \
  -ILibraries/CMSIS/Device/ST/STM32F10x/Include \
  -ILibraries/STM32F10x_StdPeriph_Driver/inc

# Compiler flags
CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections
ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

# Linker script & flags
LDSCRIPT = stm32_flash.ld
LIBS = -lc -lm -lnosys
LIBDIR =
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# -----------------------------------------------------------------------------------------
# Object files (bảo toàn cấu trúc thư mục trong build/ tránh trùng tên file)
# -----------------------------------------------------------------------------------------
OBJECTS = $(addprefix $(BUILD_DIR)/, $(C_SOURCES:.c=.o))
OBJECTS += $(addprefix $(BUILD_DIR)/, $(ASM_SOURCES:.s=.o))

# Rules
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

$(BUILD_DIR)/%.o: %.c Makefile
	@mkdir -p $(dir $@)
	@echo "CC $<"
	@$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile
	@mkdir -p $(dir $@)
	@echo "AS $<"
	@$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	@echo "LD $@"
	@$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	@echo "--- FIRMWARE SIZE ---"
	@$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf
	@$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	@$(BIN) $< $@

clean:
	-rm -fR $(BUILD_DIR)

# -----------------------------------------------------------------------------------------
# Flash & Erase
# -----------------------------------------------------------------------------------------
# Flash via OpenOCD (ST-Link)
flash: $(BUILD_DIR)/$(TARGET).elf
	openocd -f interface/stlink.cfg -f target/stm32f1x.cfg -c "program $(BUILD_DIR)/$(TARGET).elf verify reset exit"

# Flash via st-flash tool
flash_stlink: $(BUILD_DIR)/$(TARGET).bin
	st-flash write $(BUILD_DIR)/$(TARGET).bin 0x08000000

# Erase chip flash via OpenOCD
erase:
	openocd -f interface/stlink.cfg -f target/stm32f1x.cfg -c "init; reset halt; stm32f1x mass_erase 0; reset run; exit"

# Erase chip flash via st-flash
erase_stlink:
	st-flash erase

# Run OpenOCD GDB server
openocd:
	openocd -f interface/stlink.cfg -f target/stm32f1x.cfg

-include $(shell find $(BUILD_DIR) -name "*.d" 2>/dev/null)

.PHONY: all clean flash flash_stlink erase erase_stlink openocd
