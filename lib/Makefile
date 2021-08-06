CC=arm-none-eabi-gcc
AR=arm-none-eabi-ar

###########################################

vpath %.c src src/peripherals
vpath %.c src/maths

CFLAGS  = -g -O2 -Wall
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -ffreestanding -nostdlib
CFLAGS += -Iinc -Iinc/maths
CFLAGS += -D__FPU_PRESENT=1 -D__FPU_USED=1

SRCS = system_stm32f4xx.c arm_sin_f32.c arm_cos_f32.c

OBJS = $(SRCS:.c=.o)

.PHONY: libstm32f4.a

all: libstm32f4.a

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $^

libstm32f4.a: $(OBJS)
	$(AR) -r $@ $(OBJS)

clean:
	rm -f $(OBJS) libstm32f4.a
