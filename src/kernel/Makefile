# $@ = target file
# $< = first dependency
# $^ = all dependencies

ARCH := x86_64
CC   := ${ARCH}-elf-gcc
LD       = $(ARCH)-elf-ld
ASM       = $(ARCH)-elf-as
EFI_INC_DIR  := /usr/local/include/efi

INCLUDE_DIRS := ../headers

BUILD_DIR := ../../bin
SRC_DIRS := . ./cpu ./drivers ./filesystem ./libraries ./system

CCFLAGS = -m64 -O2 -g3 -ggdb -fno-omit-frame-pointer \
          -Wall -Wextra -Wpedantic -Wstrict-aliasing \
          -Wno-pointer-arith -Wno-unused-parameter \
		  -nostdlib -nostdinc -nostartfiles -nodefaultlibs \
		  -ffreestanding -fno-pie -fno-stack-protector \
          -fno-builtin-function -fno-builtin \
          -fno-pic -mno-red-zone -mno-mmx -mno-sse -mno-sse2 \
          -fno-exceptions \
          -mcmodel=large \
          $(foreach dir,$(shell find $(INCLUDE_DIRS) -type d),-I$(dir)) \
          -Wno-implicit-fallthrough -Wno-parentheses

LDFLAGS := -T ../linkers/linker.ld -ffreestanding -O2 -nostdlib -g3 \
           -z max-page-size=0x1000

KERNEL_IMG = $(BUILD_DIR)/kernel.elf

C_SRCS := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
S_SRCS := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.s))
KERNEL_OBJS= $(C_SRCS:.c=.o) $(S_SRCS:.s=.o)

# build targets
all: kernel

kernel: ${KERNEL_OBJS}
	$(CC) $(KERNEL_OBJS) $(LDFLAGS) -o $(KERNEL_IMG)

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

%.o: %.s
	$(ASM) -o $@ $<

clean:
	rm -f ${KERNEL_OBJS}