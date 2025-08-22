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
CCFLAGS = -m64 -O2 -g \
          -Wall -Wextra -Wpedantic -Wstrict-aliasing \
          -Wno-pointer-arith -Wno-unused-parameter \
          -nostdlib -nostdinc -ffreestanding -fno-pie -fno-stack-protector \
          -fno-builtin-function -fno-builtin \
          -fno-pic -mno-red-zone -mno-mmx -mno-sse -mno-sse2 \
          -nostartfiles -nodefaultlibs -fno-exceptions \
          -mcmodel=large \
          $(foreach dir,$(shell find $(C_INCLUDE) -type d),-I$(dir)) \
          -Wno-implicit-fallthrough -Wno-parentheses

KERNEL_BIN = ../../bin/kernel.bin

C_SRCS = $(wildcard $(SRC_DIR)/*.c $(SRC_DIR)/drivers/*.c $(SRC_DIR)/cpu/*.c \
                    $(SRC_DIR)/font/*.c $(SRC_DIR)/libraries/*.c $(SRC_DIR)/system/*.c \
                    $(SRC_DIR)/filesystem/*.c)
S_SRCS=$(wildcard $(SRC_DIR)/cpu/*.s)
KERNEL_OBJS= $(C_SRCS:.c=.o) $(S_SRCS:.s=.o)

# build targets
all: kernel

kernel: ${KERNEL_OBJS}
	$(LD) -o $(KERNEL_BIN) $^ $(LDFLAGS) -T../linkers/linker.ld

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

%.o: %.s
	$(ASM) -o $@ $<

clean:
	rm -f ${KERNEL_OBJS}
