# $@ = target file
# $< = first dependency
# $^ = all dependencies

TARGET   = x86_64-elf
CC       = $(TARGET)-gcc
LD       = $(TARGET)-ld
ASM       = $(TARGET)-as
C_INCLUDE := ../headers
BUILD_DIR := ../../bin
SRC_DIR = .
CCFLAGS = -m64 -O2 -g3 -ggdb -fno-omit-frame-pointer \
          -Wall -Wextra -Wpedantic -Wstrict-aliasing \
          -Wno-pointer-arith -Wno-unused-parameter \
          -nostdlib -nostdinc -ffreestanding -fno-pie -fno-stack-protector \
          -fno-builtin-function -fno-builtin \
          -fno-pic -mno-red-zone -mno-mmx -mno-sse -mno-sse2 \
          -nostartfiles -nodefaultlibs -fno-exceptions \
          -mcmodel=large \
          $(foreach dir,$(shell find $(C_INCLUDE) -type d),-I$(dir)) \
          -Wno-implicit-fallthrough -Wno-parentheses

LDFLAGS :=          \
	-ffreestanding    \
	-O2               \
	-nostdlib         \
	-z max-page-size=0x1000 \
    -g3 -ggdb

KERNEL_BIN = ../../bin/kernel.elf

C_SRCS := $(shell find $(SRC_DIR) -name '*.c')
S_SRCS := $(shell find $(SRC_DIR) -name '*.s')
KERNEL_OBJS= $(C_SRCS:.c=.o) $(S_SRCS:.s=.o)

# build targets
all: kernel

kernel: ${KERNEL_OBJS}
	$(CC) -T ../linkers/linker.ld -o $(KERNEL_BIN) $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

%.o: %.s
	$(ASM) -o $@ $<

clean:
	rm -f ${KERNEL_OBJS}
