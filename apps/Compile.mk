TARGET   = x86_64-elf
CC       = $(TARGET)-gcc
LD       = $(TARGET)-ld
ASM      = $(TARGET)-as
C_INCLUDE='../headers'
C_APP_INCLUDE='./headers'
CCFLAGS = -fno-pic  -m64 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
          -ffreestanding -mno-red-zone -mno-mmx -mno-sse -mno-sse2 \
          -I $(C_INCLUDE) -I $(C_APP_INCLUDE) -nostartfiles -nodefaultlibs -fno-exceptions \
	      -Wall -Wextra -Werror -c -mcmodel=large -Wno-parentheses -Wno-implicit-fallthrough \
	      -fdata-sections -ffunction-sections
objdump  = $(TARGET)-objdump

C_SRCS := $(wildcard *.c) $(wildcard src/*.c) $(wildcard ../lib/*.c) $(wildcard ../lib/window_api/*.c)
S_SRCS := $(wildcard ../lib/*.s)
OBJ_SRCS := $(patsubst %.c, %.o, $(C_SRCS)) $(patsubst ../lib/%.s, %.o, $(S_SRCS))

.PHONY: all clean install

all: $(name)

install:
	mkdir -p ../../../bin/apps/$(name)
	cp $(name) ../../../bin/apps/$(name)
	-cp *.kv ../../../bin/apps/$(name)
	if [ -d assets ]; then cp assets/*.bmp ../../../bin/apps/$(name); fi

clean:
	@echo "Cleaning build artifacts in $(PWD)"
	@find . -type f -name '*.o' -exec rm -f {} +
	@rm -f *.bin *.elf *.dis

debug:
	$(MAKE) CCFLAGS="$(CCFLAGS) -g" asm_flags="-g -F dwarf" all

$(name): $(OBJ_SRCS)
	$(LD) -nostdlib -n -T ../lib/linker.ld -o $(name) $(OBJ_SRCS) --gc-sections
	$(objdump) -D $(name) > ../../../bin/$(name).dump.s
	$(objdump) -x $(name) >> ../../../bin/$(name).headers.txt

%.o: ../lib/%.s
	$(ASM) -o $@ $<

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@