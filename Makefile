###################################################

SRCS = main.c codec.c timer.c rcc.c \
		iox.c spi.c i2c.c dma.c mems.c utl.c \
		 midi.c uart.c

PROJ_NAME = theremin

###################################################

CC		= arm-none-eabi-gcc
OBJCOPY	= arm-none-eabi-objcopy
GDB		= arm-none-eabi-gdb

CFLAGS  = -Os -Wall -Tstm32_flash.ld
CFLAGS += --specs=nosys.specs
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -fdata-sections -ffunction-sections
CFLAGS += -D__FPU_PRESENT=1

###################################################

vpath %.c src
vpath %.a lib

ROOT=$(shell pwd)

CFLAGS += -Iinc -Ilib -Ilib/inc
CFLAGS += -Ilib/inc/maths

SRCS += startup_stm32f4xx.s

OBJS = $(SRCS:.c=.o)

###################################################

.PHONY: lib proj

all: lib proj

lib:
	$(MAKE) -C lib

proj: 	$(PROJ_NAME).elf

$(PROJ_NAME).elf: $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ -Llib -lstm32f4 -lm -Wl,--gc-sections
	$(OBJCOPY) -O ihex $(PROJ_NAME).elf $(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin

flash:
	openocd -f board/stm32f4discovery.cfg -c \
		"init; reset halt; flash write_image erase $(PROJ_NAME).bin 0x08000000; reset run; shutdown"

debug:
	st-util & $(GDB) -silent -ex 'target extended-remote localhost:4242' $(PROJ_NAME).elf

kill:
	killall st-util

clean:
	rm -f *.o
	rm -f $(PROJ_NAME).elf
	rm -f $(PROJ_NAME).hex
	rm -f $(PROJ_NAME).bin
