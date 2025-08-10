# $@ = target file
# $< = first dependency
# $^ = all dependencies

TARGET   = i686-elf
CC       = $(TARGET)-gcc
LD       = $(TARGET)-ld
ASM       = $(TARGET)-as
CCFLAGS=-m32 -std=c11 -O2 -g -Wall -Wextra -Wpedantic -Wstrict-aliasing
CCFLAGS+=-Wno-pointer-arith -Wno-unused-parameter
CCFLAGS+=-nostdlib -nostdinc -ffreestanding -fno-pie -fno-stack-protector
CCFLAGS+=-fno-builtin-function -fno-builtin
LIBGCC := $(shell $(CC) $(CCFLAGS) -print-libgcc-file-name)

BOOTSECT_SRC=\
	src/boot/bootloader.s

BOOTSECT_OBJS=$(BOOTSECT_SRC:.s=.o)

C_SRCS = $(wildcard src/kernel/*.c src/kernel/drivers/*.c src/kernel/cpu/*.c src/kernel/font/*.c)
HEADER_SRCS = $(wildcard src/kernel/*.h  src/kernel/drivers/*.h src/kernel/cpu/*.h src/kernel/font/*.h)
S_SRCS=$(filter-out $(BOOTSECT_SRC), $(wildcard src/boot/*.s src/kernel/cpu/*.s))
OBJ_SRCS= $(C_SRCS:.c=.o) $(S_SRCS:.s=.o)

BOOTSECT=bootsect.bin
KERNEL=kernel.bin

all: dirs ${C_SRCS} ${HEADER_SRCS} bootsect kernel iso run

dirs:
	mkdir -p bin

bootsect: $(BOOTSECT_OBJS)
	$(LD) -o ./bin/$(BOOTSECT) $^ -Ttext 0x7C00 --oformat=binary

kernel: ${OBJ_SRCS}
	$(LD) -o ./bin/$(KERNEL) $^ $(LDFLAGS) $(LIBGCC) -Tsrc/linker.ld

echo: glacier-os.bin
	xxd $<

%.o: %.c ${HEADERS}
	$(CC) -g -m32 -ffreestanding -fno-pie -fno-stack-protector -c $< -o $@

%.o: %.s
	$(ASM) -o $@ $<

%.dis: %.bin
	ndisasm -b 32 $< > $@

iso: bootsect kernel
	dd if=/dev/zero of=glacier-os.iso bs=512 count=2880
	dd if=./bin/$(BOOTSECT) of=glacier-os.iso conv=notrunc bs=512 seek=0 count=1
	dd if=./bin/$(KERNEL) of=glacier-os.iso conv=notrunc bs=512 seek=1 count=2048

run: glacier-os.iso
	qemu-system-i386 -drive format=raw,file=$< -d cpu_reset -monitor stdio

clean:
	$(RM) *.bin *.o *.dis *.elf
	$(RM) bin/*.bin
	$(RM) src/kernel/*.o
	$(RM) src/boot/*.o src/boot/*.bin
	$(RM) src/kernel/drivers/*.o
	$(RM) src/kernel/cpu/*.o
	$(RM) src/kernel/font/*.o
	$(RM) *.iso