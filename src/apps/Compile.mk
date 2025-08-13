TARGET   = x86_64-elf
CC       = $(TARGET)-gcc
LD       = $(TARGET)-ld
ASM      = $(TARGET)-as
CCFLAGS  = -m32 -std=c11 -O2 -g -Wall -Wextra -Wpedantic -Wstrict-aliasing
CCFLAGS += -Wno-pointer-arith -Wno-unused-parameter
CCFLAGS += -nostdlib -nostdinc -ffreestanding -fno-pie -fno-stack-protector
CCFLAGS += -fno-builtin-function -fno-builtin
LIBGCC   := $(shell $(CC) $(CCFLAGS) -print-libgcc-file-name)

objdump  = $(TARGET)-objdump
xxd      = xxd
arch ?= x86_64-elf
linker_script := ../lib/linker.ld

C_SRCS := $(wildcard *.c) $(wildcard src/*.c) $(wildcard ../lib/*.c) $(wildcard ../lib/window_api/*.c)
S_SRCS := $(wildcard ../lib/*.s)
OBJ_SRCS := $(patsubst %.c, build/%.o, $(C_SRCS)) $(patsubst ../lib/%.asm, build/%.o, $(S_SRCS))

.PHONY: all clean run install%

all: $(name)

install%:
	mkdir -p ../../../bin/apps/$(name)
	cp $(name) ../../../bin/apps/$(name)
	-cp *.kv ../../../bin/apps/$(name)
	if [ -d assets ]; then cp assets/*.bmp ../../../bin/apps/$(name); fi

debug: nasm_flags += -g -F dwarf
debug: cflags += -g
debug: all

$(name): $(OBJ_SRCS) $(linker_script)
	$(ld) -nostdlib -n -T $(linker_script) -o $(name) $(OBJ_SRCS) --gc-sections
	$(objdump) -D $(name) > ../../../bin/$(name).dump.s
	$(objdump) -x $(name) >> ../../../bin/$(name).headers.txt

%.o: ../lib/%.s
	$(ASM) -o $@ $<

%.o: %.c
	$(CC) -g -m32 -ffreestanding -fno-pie -fno-stack-protector -c $< -o $@
