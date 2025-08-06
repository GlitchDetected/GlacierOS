TARGET = i686-elf
CC     = $(TARGET)-gcc
LD     = $(TARGET)-ld
AS     = $(TARGET)-as
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra
LDFLAGS = -T linker.ld -nostdlib

.PHONY: all clean run

all: glacieros.iso

boot.o: boot.s
	$(AS) boot.s -o boot.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

kernel.bin: boot.o kernel.o
	$(CC) $(LDFLAGS) -o myos.bin -ffreestanding -O2 -nostdlib boot.o kernel.o -lgcc

glacieros.iso: kernel.bin
	mkdir -p boot/grub
	cp kernel.bin boot/kernel.bin
	cp boot/grub.cfg boot/grub/grub.cfg
	grub-mkrescue -o glacieros.iso

run: glacieros.iso
	qemu-system-i386 -cdrom glacieros.iso

clean:
	rm -rf *.o *.bin *.iso