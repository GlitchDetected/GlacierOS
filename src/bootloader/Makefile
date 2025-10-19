.POSIX:
.DELETE_ON_ERROR:
MAKEFLAGS += --warn-undefined-variables
MAKEFLAGS += --no-builtin-rules

ARCH := x86_64
CC   := $(ARCH)-elf-gcc
LD   := $(ARCH)-elf-ld
OBJCOPY := $(ARCH)-elf-objcopy

BUILD_DIR  := ../../bin
GNU_EFI_DIR := ../../gnu-efi

EFI_INC_DIR  := ${GNU_EFI_DIR}/inc
INCLUDE_DIRS := ${EFI_INC_DIR}    \
	${EFI_INC_DIR}/${ARCH}          \
	../headers/bootloader

# The full list of includes in correct format for gcc
INCLUDE_FLAG := $(foreach d, $(INCLUDE_DIRS), -I$d)

CFLAGS := ${INCLUDE_FLAG}   \
	-ffreestanding            \
	-fno-common               \
	-fno-stack-protector      \
	-fpic                     \
	-fshort-wchar             \
	-mno-red-zone             \
	-Wall                     \
	-Wextra                   \
	-Wmissing-prototypes      \
	-Wstrict-prototypes       \
	-DEFI_FUNCTION_WRAPPER

# https://wiki.osdev.org/GNU-EFI
# git clone https://git.code.sf.net/p/gnu-efi/code gnu-efi
# cd gnu-efi
# export CROSS_COMPILE="x86_64-elf-"
# export CFLAGS='-I /usr/local/include'
# make
# sudo ARCH=x86_64 make install
LIB      := ${GNU_EFI_DIR}/${ARCH}/gnuefi
EFI_LIB      := ${GNU_EFI_DIR}/${ARCH}/lib
EFI_CRT_OBJS := ${GNU_EFI_DIR}/${ARCH}/gnuefi/crt0-efi-${ARCH}.o
EFI_LDS      := ${GNU_EFI_DIR}/gnuefi/elf_${ARCH}_efi.lds

LDFLAGS      := -nostdlib        \
	-znocombreloc              \
	-T ${EFI_LDS}              \
	-shared                    \
	-Bsymbolic                 \
	-L ${EFI_LIB}              \
	-L ${LIB} ${EFI_CRT_OBJS}

SRC_DIRS = .

C_SRCS := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
S_SRCS := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.s))

OBJECTS := ${C_SRCS:.c=.o} ${S_SRCS:.S=.o}

# The bootloader is initially compiled into a shared lib in ELF format. GNU-EFI links
# the lib in a special format as to be easily copied into a PE32+ compatible executable
# suitable for use as a UEFI bootloader.
BINARY_ELF := ${BUILD_DIR}/bootx64.so
# This is the ELF shared lib copied into the PE32+ format.
BINARY_EFI := ${BUILD_DIR}/bootx64.efi

.PHONY: all clean

all: ${BINARY_EFI}

${BINARY_EFI}: ${BINARY_ELF}
	${OBJCOPY} -j .text          \
		-j .sdata               \
		-j .data                \
		-j .dynamic             \
		-j .dynsym              \
		-j .rel                 \
		-j .rela                \
		-j .reloc               \
		--target=efi-app-${ARCH} $^ $@

${BINARY_ELF}: ${OBJECTS} ${BUILD_DIR}
	${LD} ${LDFLAGS} ${OBJECTS} -o $@ -lefi -lgnuefi

%.o: %.c
	${CC} ${CFLAGS} -o $@ -c $<

%.o: %.S
	${CC} ${CFLAGS} -o $@ -c $<

${BUILD_DIR}:
	mkdir -p ${BUILD_DIR}

clean:
	rm -f ${OBJECTS}
	rm -f ${BINARY_ELF}
	rm -f ${BINARY_EFI}
	rm -rf ${BUILD_DIR}