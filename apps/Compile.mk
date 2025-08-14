TARGET   = x86_64-elf
CC       = $(TARGET)-gcc
LD       = $(TARGET)-ld
ASM      = $(TARGET)-as
C_INCLUDE='../headers'
C_APP_INCLUDE='./headers'
CCFLAGS = -m64 -O2 -g \
          -Wall -Wextra -Wpedantic -Wstrict-aliasing \
          -Wno-pointer-arith -Wno-unused-parameter \
          -nostdlib -nostdinc -ffreestanding -fno-pie -fno-stack-protector \
          -fno-builtin-function -fno-builtin \
          -fno-pic -mno-red-zone -mno-mmx -mno-sse -mno-sse2 \
          -nostartfiles -nodefaultlibs -fno-exceptions \
          -mcmodel=large \
          -I$(C_INCLUDE) \
          -I$(C_APP_INCLUDE) \
          -Wno-implicit-fallthrough -Wno-parentheses
LIBGCC   := $(shell $(CC) $(CCFLAGS) -print-libgcc-file-name)
objdump  = $(TARGET)-objdump

C_SRCS := $(wildcard *.c) $(wildcard src/*.c) $(wildcard ../lib/*.c) $(wildcard ../lib/window_api/*.c)
S_SRCS := $(wildcard ../lib/*.s)
OBJ_SRCS := $(C_SRCS:.c=.o) $(S_SRCS:.s=.o)

.PHONY: all clean install

all: $(name)

install:
	mkdir -p ../../../bin/apps/$(name)
	cp $(name) ../../../bin/apps/$(name)
	-cp *.kv ../../../bin/apps/$(name)
	if [ -d assets ]; then cp assets/*.bmp ../../../bin/apps/$(name); fi

$(name): $(OBJ_SRCS)
	$(ld) -nostdlib -n -T ../lib/linker.ld -o $(name) $(OBJ_SRCS) --gc-sections
	$(objdump) -D $(name) > ../../../bin/$(name).dump.s
	$(objdump) -x $(name) >> ../../../bin/$(name).headers.txt

%.o: ../lib/%.s
	$(ASM) -o $@ $<

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

clean:
	$(RM) *.bin *.o *.dis *.elf
	$(RM) /*.o