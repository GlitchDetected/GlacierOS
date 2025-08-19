# $@ = target file
# $< = first dependency
# $^ = all dependencies

TARGET   = x86_64-elf
CC       = $(TARGET)-gcc
LD       = $(TARGET)-ld
ASM       = $(TARGET)-as
OBJCOPY  = $(TARGET)-objcopy
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
          $(foreach dir,$(shell find $(C_INCLUDE) -type d),-I$(dir)) \
          -Wno-implicit-fallthrough -Wno-parentheses

QEMU_MEMORY := 128
DISK_IMAGE = bin/disk.img
ISO_FILE=bin/glacier-os.iso
UEFI_BOOT_SRC = src/boot/uefi_boot.c
UEFI_BOOT_EFI = bin/EFI/BOOT/BOOT64.EFI
UEFI_BOOT_OBJ = bin/uefi_boot.o
KERNEL_BIN = bin/kernel.bin

# port contents qemu | grep fd
# Learn more here: https://github.com/tianocore/tianocore.github.io/wiki/OVMF 
# https://wiki.osdev.org/OVMF
# https://github.com/tianocore/edk2/tree/master/OvmfPkg 
OVMF_CODE := /opt/local/share/qemu/edk2-x86_64-code.fd
BIN_OVMF := bin/edk2-x86_64-code.fd

C_SRCS = $(wildcard src/kernel/*.c src/kernel/drivers/*.c src/kernel/cpu/*.c src/kernel/font/*.c src/kernel/libraries/*.c src/kernel/system/*.c src/kernel/filesystem/*.c)
S_SRCS=$(wildcard src/boot/*.s src/kernel/cpu/*.s)
KERNEL_OBJS= $(C_SRCS:.c=.o) $(S_SRCS:.s=.o)

all: dirs ${C_SRCS} uefi_boot kernel appsDir install

appsDir:
	$(MAKE) -C apps all

dirs:
	mkdir -p bin
	mkdir -p bin/disk
	mkdir -p bin/EFI/BOOT

uefi_boot: $(UEFI_BOOT_SRC)
	$(CC) $(CCFLAGS) -fshort-wchar -c $< -o bin/uefi_boot.o
	$(LD) -nostdlib -znocombreloc -shared -Bsymbolic \
		-Tsrc/linkers/elf_x86_64_efi.lds \
		$(UEFI_BOOT_OBJ) -o $(UEFI_BOOT_EFI)

kernel: ${KERNEL_OBJS}
	$(LD) -o $(KERNEL_BIN) $^ $(LDFLAGS) -Tsrc/linkers/linker.ld

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

%.o: %.s
	$(ASM) -o $@ $<

install: uefi_boot kernel
	# make the fat32 disk image
	dd if=/dev/zero of=$(DISK_IMAGE) count=64 bs=1M
	# mkfs.fat needs dosfstools (install it with macports or homebrew)
	mkfs.fat -F 32 $(DISK_IMAGE)
	mmd -i $(DISK_IMAGE) ::/EFI
	mmd -i $(DISK_IMAGE) ::/EFI/BOOT

	# copy EFI bootloader and kernel
	mcopy -i $(DISK_IMAGE) $(UEFI_BOOT_EFI) ::/EFI/BOOT/BOOTX64.EFI
	mcopy -i $(DISK_IMAGE) $(KERNEL_BIN) ::/

	# make iso from the fat32 disk image
	dd if=$(DISK_IMAGE) of=$(ISO_FILE) bs=512 conv=notrunc

debug:
	$(MAKE) CCFLAGS="$(CCFLAGS) -g" asm_flags="-g -F dwarf" all

$(BIN_OVMF):
	cp $(OVMF_CODE) $(BIN_OVMF)
	chmod +r $(BIN_OVMF)

run: $(ISO_FILE) $(BIN_OVMF)
	$(QEMU_ARCH) -bios $(BIN_OVMF) \
	  -cdrom $(ISO_FILE) -m $(QEMU_MEMORY) \
	  -serial stdio

clean:
	@echo "Cleaning build artifacts..."
	@find . -type f -name '*.o' -exec rm -f {} +
	@rm -rf bin/*
	@rm -f *.bin *.o *.dis *.elf *.iso
	$(MAKE) -C apps clean
