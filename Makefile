TARGET   = i686-elf
CC       = $(TARGET)-gcc
LD       = $(TARGET)-ld
AS       = $(TARGET)-as
CFLAGS   = -std=gnu99 -ffreestanding -O2 -Wall -Wextra
LDFLAGS  = -T linker.ld -nostdlib

OUTPUT_DIR = build
LIMINE_DIR = $(OUTPUT_DIR)/limine

.PHONY: all clean run dirs install-deps

all: dirs $(OUTPUT_DIR)/glacieros.iso

dirs:
	mkdir -p $(OUTPUT_DIR)

$(OUTPUT_DIR)/boot.o: boot.s | dirs
	$(AS) boot.s -o $@

$(OUTPUT_DIR)/kernel.o: kernel.c | dirs
	$(CC) $(CFLAGS) -c kernel.c -o $@

$(OUTPUT_DIR)/kernel.bin: $(OUTPUT_DIR)/boot.o $(OUTPUT_DIR)/kernel.o
	$(CC) $(LDFLAGS) -o $(OUTPUT_DIR)/kernel.bin -ffreestanding -O2 -nostdlib $^ -lgcc

$(OUTPUT_DIR)/glacieros.iso: $(OUTPUT_DIR)/kernel.bin
	mkdir -p $(OUTPUT_DIR)/boot/

	cp $(OUTPUT_DIR)/kernel.bin $(OUTPUT_DIR)/boot/kernel.bin
	cp boot/limine.cfg $(OUTPUT_DIR)/
	cp $(LIMINE_DIR)/limine.sys $(OUTPUT_DIR)/
	cp $(LIMINE_DIR)/limine.bin $(OUTPUT_DIR)/

	xorriso -as mkisofs \
		-b limine.bin \
		-c limine.sys \
		-no-emul-boot \
		-boot-load-size 4 \
		-boot-info-table \
		-o $@ \
		$(OUTPUT_DIR)

	$(LIMINE_DIR)/limine-install $@

run: $(OUTPUT_DIR)/glacieros.iso
	qemu-system-i386 -cdrom $<

clean:
	rm -rf $(OUTPUT_DIR)

install-deps:
	@echo "[*] Installing dependencies..."

	@if command -v port >/dev/null 2>&1; then \
	    echo "[*] Using MacPorts..."; \
		sudo port install coreutils gmake grep gsed findutils gawk gzip nasm mtools qemu xorriso; \
	elif command -v brew >/dev/null 2>&1; then \
		echo "[*] Using Homebrew..."; \
		brew install coreutils make grep gnu-sed findutils gawk gzip nasm mtools qemu xorriso; \
	else \
		echo "[!] No supported package manager found (Homebrew or MacPorts). Please install manually."; \
		exit 1; \
	fi

	@if [ ! -d "$(LIMINE_DIR)" ]; then \
		echo "[*] Cloning Limine binary release to $(LIMINE_DIR)..."; \
		git clone --branch=v9.x-binary --depth=1 https://github.com/limine-bootloader/limine.git $(LIMINE_DIR); \
	else \
		echo "[*] Limine binary release already exists in $(LIMINE_DIR), skipping clone."; \
	fi