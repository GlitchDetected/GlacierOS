# $@ = target file
# $< = first dependency
# $^ = all dependencies

TARGET   = x86_64-elf
CC       = $(TARGET)-gcc
LD       = $(TARGET)-ld
ASM       = $(TARGET)-as
QEMU_ARCH = qemu-system-x86_64
C_INCLUDE := 'src/headers'
CCFLAGS = -m64 -O2 -g \
          -Wall -Wextra -Wpedantic -Wstrict-aliasing \
          -Wno-pointer-arith -Wno-unused-parameter \
          -nostdlib -nostdinc -ffreestanding -fno-pie -fno-stack-protector \
          -fno-builtin-function -fno-builtin \
          -fno-pic -mno-red-zone -mno-mmx -mno-sse -mno-sse2 \
          -nostartfiles -nodefaultlibs -fno-exceptions \
          -mcmodel=large \
          -I$(C_INCLUDE) \
          -Wno-implicit-fallthrough -Wno-parentheses
QEMU_MEMORY := 128
DISK_IMAGE = bin/disk.img
ISO_FILE=bin/glacier-os.iso

BOOTSECT_SRC=\
	src/boot/bootloader.s
BOOTSECT_OBJS=$(BOOTSECT_SRC:.s=.o)

C_SRCS = $(wildcard src/kernel/*.c src/kernel/drivers/*.c src/kernel/cpu/*.c src/kernel/font/*.c src/kernel/libraries/*.c src/kernel/system/*.c src/kernel/filesystem/*.c)
S_SRCS=$(filter-out $(BOOTSECT_SRC), $(wildcard src/boot/*.s src/kernel/cpu/*.s))
KERNEL_OBJS= $(C_SRCS:.c=.o) $(S_SRCS:.s=.o)

BOOTSECT=bootsect.bin
KERNEL=kernel.bin

all: dirs ${C_SRCS} bootsect kernel appsDir install

appsDir:
	$(MAKE) -C apps all

dirs:
	mkdir -p bin
	mkdir -p bin/disk

bootsect: $(BOOTSECT_OBJS)
	$(LD) -o ./bin/$(BOOTSECT) $^ -Ttext 0x7C00 --oformat=binary

kernel: ${KERNEL_OBJS}
	$(LD) -o ./bin/$(KERNEL) $^ $(LDFLAGS) -Tsrc/linker.ld

echo:
	xxd bin/images/glacier-os.bin

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

%.o: %.s
	$(ASM) -o $@ $<

install: bootsect kernel
	dd if=/dev/zero of=$(DISK_IMAGE) count=81920 bs=512
	mformat -F -v GLACIER-DISK -i $(DISK_IMAGE)
	mcopy -svn -i $(DISK_IMAGE) $(wildcard bin/disk/*) ::.
	dd if=/dev/zero of=$(ISO_FILE) bs=512 count=2880
	dd if=./bin/$(BOOTSECT) of=$(ISO_FILE) conv=notrunc bs=512 seek=0 count=1
	dd if=./bin/$(KERNEL) of=$(ISO_FILE) conv=notrunc bs=512 seek=1 count=2048

debug:
	$(MAKE) CCFLAGS="$(CCFLAGS) -g" asm_flags="-g -F dwarf" all

run: $(ISO_FILE)
	$(QEMU_ARCH) -cdrom $(ISO_FILE) -m $(QEMU_MEMORY) -drive file=$(DISK_IMAGE),format=raw,index=0,media=disk  -boot order=d -serial stdio

run-debug: $(ISO_FILE)
	$(QEMU_ARCH) -cdrom $(ISO_FILE) -m $(QEMU_MEMORY) -drive file=$(DISK_IMAGE),format=raw,index=0,media=disk  -boot order=d -s -S

clean:
	@echo "Cleaning build artifacts..."
	@find . -type f -name '*.o' -exec rm -f {} +
	@rm -rf bin/*
	@rm -f *.bin *.o *.dis *.elf *.iso
	$(MAKE) -C apps clean