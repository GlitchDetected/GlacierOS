#ifndef KERNEL_H
#define KERNEL_H
#include <stdint.h>

typedef uint64_t UINTN;
typedef uint32_t UINT32;
typedef uint64_t EFI_PHYSICAL_ADDRESS;
typedef void EFI_RUNTIME_SERVICES;

/* Memory descriptor structure */
typedef struct {
    UINT32 Type;
    EFI_PHYSICAL_ADDRESS PhysicalStart;
    EFI_PHYSICAL_ADDRESS VirtualStart;
    UINTN NumberOfPages;
    UINTN Attribute;
} EFI_MEMORY_DESCRIPTOR;

/* Graphics output protocol mode info */
typedef struct {
    UINT32 Version;
    UINT32 HorizontalResolution;
    UINT32 VerticalResolution;
    UINT32 PixelsPerScanLine;
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;

typedef struct {
    UINT32 MaxMode;
    UINT32 Mode;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
    UINTN SizeOfInfo;
    EFI_PHYSICAL_ADDRESS FrameBufferBase;
    UINTN FrameBufferSize;
} EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;

/*-----------------------------------
  Kernel-specific types
-----------------------------------*/
typedef struct {
    UINTN mm_size;
    EFI_MEMORY_DESCRIPTOR *mm_descriptor;
    UINTN map_key;
    UINTN descriptor_size;
    UINT32 descriptor_version;
} memory_map_t;

typedef struct {
    memory_map_t mm;
    EFI_RUNTIME_SERVICES *runtime_services;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE graphic_out_protocol;
    UINTN custom_protocol_data;
} boot_params_t;

typedef void (*kernel_entry)(boot_params_t *params);

#define KERNEL_START_MEMORY 0xFFFF800000000000
#define KERNEL_VMA ((char*)KERNEL_START_MEMORY)

#define KERNEL_USER_VIEW_START_MEMORY 0xFFFFE00000000000
#define KERNEL_USER_VIEW_VMA ((char*)KERNEL_USER_VIEW_START_MEMORY)

#define KERNEL_VIDEO_MEMORY ((unsigned char*)0xFFFFC00000000000)

#define TO_VMA_U64(ptr) ((uint64_t)ptr + (uint64_t)KERNEL_START_MEMORY)
#define TO_VMA_PTR(type, ptr) ((type)TO_VMA_U64(ptr))
#define TO_PHYS_U64(ptr) ((uint64_t)ptr ^ (uint64_t)KERNEL_START_MEMORY)
#define TO_PHYS_PTR(type, ptr) ((type)TO_PHYS_U64(ptr))

#define GDT_NULL        0x00
#define GDT_KERNEL_CODE 0x08
#define GDT_KERNEL_DATA 0x10
#define GDT_USER_CODE   0x18
#define GDT_USER_DATA   0x20
#define GDT_TSS         0x28

void HALT_AND_CATCH_FIRE(const char *fmt, ...);
void _kernel_debug(const char *fmt, ...);
#define __UNUSED__ __attribute__((unused))

#define DEBUG _kernel_debug

#endif