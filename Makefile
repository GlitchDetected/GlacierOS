# $@ = target file
# $< = first dependency
# $^ = all dependencies

ARCH := x86_64
CC   := ${ARCH}-elf-gcc
LD       = $(ARCH)-elf-ld
ASM       = $(ARCH)-elf-as
QEMU_ARCH = qemu-system-x86_64

DISK_IMG_SIZE := 262144
DISK_IMG = bin/disk.img
ISO_FILE=bin/glacier-os.iso

BUILD_DIR         := bin

BOOTLOADER_DIR    := src/bootloader
BOOTLOADER_BINARY := bin/bootx64.efi

KERNEL_DIR := src/kernel
KERNEL_BIN := bin/kernel.elf

# port contents qemu | grep fd
# Learn more here: https://github.com/tianocore/tianocore.github.io/wiki/OVMF 
# https://wiki.osdev.org/OVMF
# https://github.com/tianocore/edk2/tree/master/OvmfPkg 
OVMF_CODE := /opt/local/share/qemu/edk2-x86_64-code.fd
OVMF_VARS := bin/edk2-x86_64-vars.fd

QEMU_FLAGS :=                                                \
	-bios ${OVMF_CODE}                                        \
	-pflash ${OVMF_VARS} \
	-drive if=none,id=uas-disk1,file=${DISK_IMG},format=raw    \
	-device usb-storage,drive=uas-disk1                        \
	-serial stdio                                              \
	-usb                                                       \
	-net none                                                  \
	-vga std

all: dirs bootloader kernel appsDir install

bootloader:
	make -C $(BOOTLOADER_DIR)

kernel:
	make -C $(KERNEL_DIR)

appsDir:
	$(MAKE) -C apps all

lib:
	cd gnu-efi && \
	export CROSS_COMPILE="${ARCH}-elf-" && \
	export CC="${ARCH}-elf-gcc" && \
	export AS="${ARCH}-elf-as" && \
	export AR="${ARCH}-elf-ar" && \
	export LD="${ARCH}-elf-ld" && \
	export OBJCOPY="${ARCH}-elf-objcopy" && \
	export CFLAGS="-I/usr/local/include" && \
	$(MAKE)

dirs:
	mkdir -p bin
	mkdir -p bin/disk

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

%.o: %.s
	$(ASM) -o $@ $<

install: 
	# Create UEFI boot disk image in DOS format
	dd if=/dev/zero of=${DISK_IMG} bs=512 count=${DISK_IMG_SIZE}

	mformat -i ${DISK_IMG} ::

	# Create EFI/BOOT directories
	mmd -i ${DISK_IMG} ::/EFI
	mmd -i ${DISK_IMG} ::/EFI/BOOT

	# Copy the bootloader and kernel to the boot partition
	mcopy -i ${DISK_IMG} ${BOOTLOADER_BINARY} ::/EFI/BOOT/bootx64.EFI
	mcopy -i ${DISK_IMG} ${KERNEL_BIN} ::/kernel.elf

${OVMF_VARS}:
	cp $(OVMF_CODE) $(OVMF_VARS)
	chmod +w $(OVMF_VARS)

debug:
	greadelf -h bin/kernel.elf
	greadelf -l bin/kernel.elf
	x86_64-elf-addr2line -e bin/kernel.elf -f -C

run-debug: ${DISK_IMG} ${OVMF_VARS}
	qemu-system-x86_64    \
		${QEMU_FLAGS}       \
		-S                  \
		-gdb tcp::1234

run: ${DISK_IMG} ${OVMF_VARS}
	qemu-system-x86_64    \
		${QEMU_FLAGS}

clean:
	make clean -C ${BOOTLOADER_DIR}
	make clean -C ${KERNEL_DIR}
	make clean -C apps
	rm -rf ${BUILD_DIR}