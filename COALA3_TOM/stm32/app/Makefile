PROJ_NAME = coala
LINKER_SCRIPT = STM32F030K6_FLASH.ld
STARTUP_OBJ = startup_stm32f030x6.o
STAT_LIBS = stlib.a
###################################################
# Set toolchain
TOOLCHAIN = ~/stm32/gcc-arm-none-eabi/bin/
PFIX = arm-none-eabi
TC=$(TOOLCHAIN)$(PFIX)
export PATH:=$(PATH):$(TOOLCHAIN)

# Set Tools
CC			= $(TC)-gcc
AR			= $(TC)-ar
OBJCOPY		= $(TC)-objcopy
OBJDUMP		= $(TC)-objdump
SIZE		= $(TC)-size
GDB			= $(TC)-gdb

# Set Libraries root
LIB_ROOT = ../libs/stlib

###################################################
# Set Sources
LIB_SRCS	= $(wildcard $(LIB_ROOT)/STM32F0xx_HAL_Driver/Src/*.c)
USER_SRCS	= $(wildcard *.c)

# Set Objects
LIB_OBJS	= $(LIB_SRCS:.c=.o)
USER_OBJS	= $(USER_SRCS:.c=.o) $(STARTUP_OBJ)

# Set Include Paths
INCLUDES 	= -I$(LIB_ROOT)/STM32F0xx_HAL_Driver/Inc \
			 -I$(LIB_ROOT)/CMSIS/Device/ST/STM32F0xx/Include/ -I$(LIB_ROOT)/CMSIS/Include -I./
			
# Set Libraries
LIBS		= -lm -lc

###################################################
# Set Board
MCU 		= -mthumb -mcpu=cortex-m0 -fno-common
DEFINES 	= -DSTM32F030x6 -DUSE_STDPERIPH_DRIVER

# Set Compilation and Linking Flags
CFLAGS 		= $(MCU) $(DEFINES) $(INCLUDES) \
			-g -Wall -std=gnu90 -O2 -ffunction-sections -fdata-sections
ASFLAGS 	= $(MCU) -g -Wa,--warn -x assembler-with-cpp
LDFLAGS 	= $(MCU) -g -gdwarf-2\
			-T$(LINKER_SCRIPT)\
			-Xlinker --gc-sections -Wl,-Map=$(PROJ_NAME).map \
			$(LIBS) \
			-o $(PROJ_NAME).elf

###################################################
# Default Target
all: $(PROJ_NAME).bin info

# elf Target
$(PROJ_NAME).elf: $(USER_OBJS)
	$(CC) $(USER_OBJS) $(STAT_LIBS) $(LDFLAGS)
	@echo $@

# bin Target
$(PROJ_NAME).bin: $(PROJ_NAME).elf
	@$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin
	@echo $@

# Display Memory Usage Info
info: $(PROJ_NAME).elf
	@$(SIZE) --format=berkeley $(PROJ_NAME).elf

# Build of stlib, forced to rebuild all each time 
.PHONY: rebuild
stlib.a: rebuild $(LIB_OBJS)
	$(AR) rcs $@ $(LIB_OBJS)

rebuild:
	@rm -f $(LIB_OBJS)

# Rule for .c files
.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<
	@echo $@

# Rule for .s files
.s.o:
	@$(CC) $(ASFLAGS) -c -o $@ $<
	@echo $@

# Clean Target
clean:
	rm -f $(USER_OBJS)
	rm -f $(PROJ_NAME).elf
	rm -f $(PROJ_NAME).bin
	rm -f $(PROJ_NAME).map

cleanall: clean
	rm -f $(LIB_OBJS)
	rm -f stlib.a

debug:
	$(GDB) --eval-command="target remote localhost:3333" $(PROJ_NAME).elf

flash:
#	./stm32flash -b 115200 -k /dev/ttyUSB0
	./stm32flash -b 115200 -w $(PROJ_NAME).bin -v /dev/ttyUSB0
#	./stm32flash -b 115200 -j /dev/ttyUSB0

show:
	echo $(LIB_OBJS)
