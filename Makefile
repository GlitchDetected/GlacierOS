# $@ = target file
# $< = first dependency
# $^ = all dependencies

TARGET   = x86_64-elf
CC       = $(TARGET)-gcc
LD       = $(TARGET)-ld
ASM       = $(TARGET)-as
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
LIBGCC := $(shell $(CC) $(CCFLAGS) -print-libgcc-file-name)

BOOTSECT_SRC=\
	src/boot/bootloader.s

BOOTSECT_OBJS=$(BOOTSECT_SRC:.s=.o)

C_SRCS = $(wildcard src/kernel/*.c src/kernel/drivers/*.c src/kernel/cpu/*.c src/kernel/font/*.c src/kernel/libraries/*.c src/kernel/system/*.c src/kernel/filesystem/*.c)
S_SRCS=$(filter-out $(BOOTSECT_SRC), $(wildcard src/boot/*.s src/kernel/cpu/*.s))
OBJ_SRCS= $(C_SRCS:.c=.o) $(S_SRCS:.s=.o)

BOOTSECT=bootsect.bin
KERNEL=kernel.bin

all: dirs ${C_SRCS} bootsect kernel apps iso

apps:
	$(MAKE) -C src/apps all

dirs:
	mkdir -p bin

bootsect: $(BOOTSECT_OBJS)
	$(LD) -o ./bin/$(BOOTSECT) $^ -Ttext 0x7C00 --oformat=binary

kernel: ${OBJ_SRCS}
	$(LD) -o ./bin/$(KERNEL) $^ $(LDFLAGS) $(LIBGCC) -Tsrc/linker.ld

echo: glacier-os.bin
	xxd $<

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

%.o: %.s
	$(ASM) -o $@ $<

%.dis: %.bin
	ndisasm -b 64 $< > $@

iso: bootsect kernel
	dd if=/dev/zero of=glacier-os.iso bs=512 count=2880
	dd if=./bin/$(BOOTSECT) of=glacier-os.iso conv=notrunc bs=512 seek=0 count=1
	dd if=./bin/$(KERNEL) of=glacier-os.iso conv=notrunc bs=512 seek=1 count=2048

run: glacier-os.iso
	qemu-system-x86_64 -drive format=raw,file=$< -d cpu_reset -monitor stdio

clean:
	$(RM) *.bin *.o *.dis *.elf
	$(RM) bin/*.bin
	$(RM) src/kernel/*.o
	$(RM) src/boot/*.o src/boot/*.bin
	$(RM) src/kernel/drivers/*.o
	$(RM) src/kernel/cpu/*.o
	$(RM) src/kernel/font/*.o
	$(RM) src/kernel/libraries/*.o
	$(RM) src/kernel/system/*.o
	$(RM) src/kernel/filesystem/*.o
	$(RM) *.iso