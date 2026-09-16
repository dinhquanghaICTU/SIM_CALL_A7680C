##########################################################################################################################
# Makefile for STM32F103C8T6 with Standard Peripheral Library (StdPeriph)
##########################################################################################################################

TARGET = stm32f103_stdperiph
DEBUG = 1
OPT = -Og

BUILD_DIR = build

# C Sources
C_SOURCES = \
User/main.c \
User/stm32f10x_it.c \
Startup/system_stm32f10x.c \
Libraries/STM32F10x_StdPeriph_Driver/src/misc.c \
Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_adc.c \
Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_bkp.c \
Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_can.c \
Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_cec.c \
Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_crc.c \
Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_dbgmcu.c \
Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_dma.c \
Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_exti.c \
Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_flash.c \
Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c \
Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_i2c.c \
Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_iwdg.c \
Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_pwr.c \
Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c \
Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_rtc.c \
Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_spi.c \
Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_tim.c \
Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_usart.c \
Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_wwdg.c

# ASM Sources
ASM_SOURCES = \
Startup/startup_stm32f10x_md.s

# Toolchain
PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

# MCU Architecture
CPU = -mcpu=cortex-m3
MCU = $(CPU) -mthumb

# C Defines
C_DEFS = \
-DSTM32F10X_MD \
-DUSE_STDPERIPH_DRIVER

# C Includes
C_INCLUDES = \
-IUser \
-IStartup \
-ILibraries/CMSIS/Include \
-ILibraries/CMSIS/Device/ST/STM32F10x/Include \
-ILibraries/STM32F10x_StdPeriph_Driver/inc

# Compiler Flags
CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections
ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

# Linker Script
LDSCRIPT = stm32_flash.ld

# Linker Flags
LIBS = -lc -lm -lnosys
LIBDIR =
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# Objects list
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

# Rules
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	@echo "CC $<"
	@$(CC) -c $(CFLAGS) -Wa,-a,-ad,-gnms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	@echo "AS $<"
	@$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	@echo "LD $@"
	@$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	@echo "--- FIRMWARE SIZE ---"
	@$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@$(BIN) $< $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	-rm -fR $(BUILD_DIR)

# Flash via OpenOCD (ST-Link)
flash: $(BUILD_DIR)/$(TARGET).elf
	openocd -f interface/stlink.cfg -f target/stm32f1x.cfg -c "program $(BUILD_DIR)/$(TARGET).elf verify reset exit"

# Flash via st-flash tool
flash_stlink: $(BUILD_DIR)/$(TARGET).bin
	st-flash write $(BUILD_DIR)/$(TARGET).bin 0x08000000

# Run OpenOCD GDB server
openocd:
	openocd -f interface/stlink.cfg -f target/stm32f1x.cfg

-include $(wildcard $(BUILD_DIR)/*.d)

.PHONY: all clean flash flash_stlink openocd
