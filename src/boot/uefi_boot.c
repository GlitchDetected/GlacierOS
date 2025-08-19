#include <stdint.h>
#include <efi.h>
#include <efilib.h>

#define EI_NIDENT 16
#define PT_LOAD 1

typedef struct {
    unsigned char e_ident[EI_NIDENT];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf64_Ehdr;

typedef struct {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
} Elf64_Phdr;

// --- Paging bits ---
#define PTE_P (1ULL<<0)
#define PTE_RW (1ULL<<1)
#define PTE_US (1ULL<<2)
#define PTE_PWT (1ULL<<3)
#define PTE_PCD (1ULL<<4)
#define PTE_A (1ULL<<5)
#define PTE_D (1ULL<<6)
#define PTE_PS (1ULL<<7)
#define PTE_G (1ULL<<8)
#define PTE_XD (1ULL<<63)

#define ALIGN_UP(x,a) (((x)+((a)-1)) & ~((a)-1))
#define ALIGN_DOWN(x,a) ((x) & ~((a)-1))

typedef struct {
    uint64_t rsdp;
    uint64_t fb_base;
    uint32_t fb_width;
    uint32_t fb_height;
    uint32_t fb_pitch;
    uint32_t fb_bpp;
    uint64_t mmap;
    uint64_t mmap_size;
    uint64_t mmap_desc_size;
    uint32_t mmap_desc_version;
} boot_info_t;

typedef struct {
    uint64_t va;
    uint64_t pa;
    uint64_t size;
    uint64_t flags;
} map_range_t;

#define KERNEL_LOAD_ADDR 0x100000ULL
#define KERNEL_VMA 0xFFFF800000000000ULL

static uint64_t __attribute__((aligned(0x1000))) PML4[512];

static EFI_PHYSICAL_ADDRESS alloc_page(EFI_BOOT_SERVICES* BS) {
    EFI_PHYSICAL_ADDRESS addr = 0;
    EFI_STATUS st = BS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &addr);
    if (EFI_ERROR(st)) {
        Print(L"AllocatePages failed: %r\n", st);
        for(;;){}
    }
    return addr;
}

static void map_4k(EFI_BOOT_SERVICES* BS, uint64_t pml4_phys, uint64_t va, uint64_t pa, uint64_t flags) {
    uint64_t *pml4 = (uint64_t*)pml4_phys;
    uint16_t i4 = (va >> 39) & 0x1FF;
    uint16_t i3 = (va >> 30) & 0x1FF;
    uint16_t i2 = (va >> 21) & 0x1FF;
    uint16_t i1 = (va >> 12) & 0x1FF;

    if (!(pml4[i4] & PTE_P)) {
        EFI_PHYSICAL_ADDRESS pdpt = alloc_page(BS);
        for (int i=0;i<512;i++) ((uint64_t*)pdpt)[i] = 0;
        pml4[i4] = (pdpt & ~0xFFFULL) | PTE_P | PTE_RW | PTE_US;
    }
    uint64_t *pdpt = (uint64_t*)(pml4[i4] & ~0xFFFULL);

    if (!(pdpt[i3] & PTE_P)) {
        EFI_PHYSICAL_ADDRESS pd = alloc_page(BS);
        for (int i=0;i<512;i++) ((uint64_t*)pd)[i] = 0;
        pdpt[i3] = (pd & ~0xFFFULL) | PTE_P | PTE_RW | PTE_US;
    }
    uint64_t *pd = (uint64_t*)(pdpt[i3] & ~0xFFFULL);

    if (!(pd[i2] & PTE_P)) {
        EFI_PHYSICAL_ADDRESS pt = alloc_page(BS);
        for (int i=0;i<512;i++) ((uint64_t*)pt)[i] = 0;
        pd[i2] = (pt & ~0xFFFULL) | PTE_P | PTE_RW | PTE_US;
    }
    uint64_t *pt = (uint64_t*)(pd[i2] & ~0xFFFULL);

    pt[i1] = (pa & ~0xFFFULL) | (flags & ~(PTE_PS));
}

static void map_range_4k(EFI_BOOT_SERVICES* BS, uint64_t pml4_phys, map_range_t r) {
    uint64_t off = 0;
    while (off < r.size) {
        map_4k(BS, pml4_phys, r.va + off, r.pa + off, r.flags);
        off += 0x1000ULL;
    }
}

static uint64_t find_rsdp(EFI_SYSTEM_TABLE *ST) {
    static EFI_GUID Acpi20 = { 0x8868e871,0xe4f1,0x11d3,{0xbc,0x22,0x00,0x80,0xc7,0x3c,0x88,0x81} };
    static EFI_GUID Acpi10 = { 0xeb9d2d30,0x2d88,0x11d3,{0x9a,0x16,0x00,0x90,0x27,0x3f,0xc1,0x4d} };
    for (UINTN i=0;i<ST->NumberOfTableEntries;i++) {
        EFI_CONFIGURATION_TABLE *ct = &ST->ConfigurationTable[i];
        if (!CompareGuid(&ct->VendorGuid, &Acpi20) || !CompareGuid(&ct->VendorGuid, &Acpi10)) {
            return (uint64_t)(uintptr_t)ct->VendorTable;
        }
    }
    return 0;
}

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);
    EFI_BOOT_SERVICES *BS = SystemTable->BootServices;
    boot_info_t boot = {0};
    Print(L"UEFI bootloader starting...\n");

    // --- Open root volume and kernel file ---
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Volume;
    EFI_FILE_PROTOCOL *Root, *KernelFile;
    if (EFI_ERROR(BS->LocateProtocol(&gEfiSimpleFileSystemProtocolGuid, NULL, (void**)&Volume))) return EFI_LOAD_ERROR;
    if (EFI_ERROR(Volume->OpenVolume(Volume, &Root))) return EFI_LOAD_ERROR;
    if (EFI_ERROR(Root->Open(Root, &KernelFile, L"kernel.bin", EFI_FILE_MODE_READ, 0))) return EFI_NOT_FOUND;

    // --- Get kernel size ---
    UINTN kernel_size;
    if (EFI_ERROR(KernelFile->SetPosition(KernelFile, 0))) return EFI_LOAD_ERROR;
    EFI_FILE_INFO *info;
    UINTN info_size = sizeof(EFI_FILE_INFO) + 200;
    if (EFI_ERROR(BS->AllocatePool(EfiLoaderData, info_size, (void**)&info))) return EFI_OUT_OF_RESOURCES;
    if (EFI_ERROR(KernelFile->GetInfo(KernelFile, &gEfiFileInfoGuid, &info_size, info))) return EFI_LOAD_ERROR;
    kernel_size = info->FileSize;

    // --- Allocate pages for kernel ---
    UINT64 kernel_pages = ALIGN_UP(kernel_size, 0x1000) >> 12;
    EFI_PHYSICAL_ADDRESS kernel_phys = 0;
    if (EFI_ERROR(BS->AllocatePages(AllocateAnyPages, EfiLoaderData, kernel_pages, &kernel_phys))) return EFI_OUT_OF_RESOURCES;

    // --- Read kernel into memory ---
    if (EFI_ERROR(KernelFile->SetPosition(KernelFile, 0))) return EFI_LOAD_ERROR;
    UINTN to_read = kernel_size;
    if (EFI_ERROR(KernelFile->Read(KernelFile, &to_read, (void*)(uintptr_t)kernel_phys)) || to_read != kernel_size) return EFI_LOAD_ERROR;

    // --- Prepare page tables ---
    EFI_PHYSICAL_ADDRESS pml4_phys = alloc_page(BS);
    for (int i=0;i<512;i++) ((uint64_t*)pml4_phys)[i] = 0;

    // Identity-map low memory
    map_range_t ident = { .va = 0x0, .pa = 0x0, .size = 16*1024*1024, .flags = PTE_P|PTE_RW };
    map_range_4k(BS, pml4_phys, ident);

    // Map kernel at physical address with virtual mapping
    map_range_t kernel_map = { .va = KERNEL_VMA, .pa = kernel_phys, .size = ALIGN_UP(kernel_size, 0x1000), .flags = PTE_P|PTE_RW|PTE_XD };
    map_range_4k(BS, pml4_phys, kernel_map);

    // --- Create kernel stack ---
    EFI_PHYSICAL_ADDRESS stack_phys = 0;
    BS->AllocatePages(AllocateAnyPages, EfiLoaderData, 16, &stack_phys);
    uint64_t stack_top = stack_phys + 16*4096ULL;
    map_range_t kstack = { .va = stack_phys, .pa = stack_phys, .size = 16*4096ULL, .flags = PTE_P|PTE_RW|PTE_XD };
    map_range_4k(BS, pml4_phys, kstack);

    // --- Initialize maximum resolution framebuffer ---
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = NULL;
    if (!EFI_ERROR(BS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (void**)&gop))) {
        UINT32 max_mode = 0;
        UINT32 max_width = 0, max_height = 0;
        for (UINT32 m=0;m<gop->Mode->MaxMode;m++) {
            EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
            UINTN sz;
            if (EFI_ERROR(gop->QueryMode(gop, m, &sz, &info))) continue;
            if (info->HorizontalResolution*info->VerticalResolution > max_width*max_height) {
                max_mode = m;
                max_width = info->HorizontalResolution;
                max_height = info->VerticalResolution;
            }
        }
        gop->SetMode(gop, max_mode);

        UINT64 fb_phys = gop->Mode->FrameBufferBase;
        UINT64 fb_size = gop->Mode->FrameBufferSize;
        map_range_t fb_map = { .va = KERNEL_VMA + 0x80000000, .pa = fb_phys, .size = ALIGN_UP(fb_size, 0x1000), .flags = PTE_P|PTE_RW };
        map_range_4k(BS, pml4_phys, fb_map);

        boot.fb_base   = fb_map.va;
        boot.fb_width  = gop->Mode->Info->HorizontalResolution;
        boot.fb_height = gop->Mode->Info->VerticalResolution;
        boot.fb_pitch  = gop->Mode->Info->PixelsPerScanLine * 4;
        boot.fb_bpp    = 32;

        UINT32 *fb = (UINT32*)(uintptr_t)boot.fb_base;
        UINTN pixels = boot.fb_width*boot.fb_height;
        for (UINTN i=0;i<pixels;i++) fb[i]=0x00000000;

        Print(L"Framebuffer mapped at %p, resolution: %ux%u\n", fb_map.va, boot.fb_width, boot.fb_height);
    }

    // --- ACPI / memory map / exit boot services ---
    boot.rsdp = find_rsdp(SystemTable);
    UINTN mmap_size = 0, map_key = 0, desc_size = 0; UINT32 desc_ver = 0;
    BS->GetMemoryMap(&mmap_size, NULL, &map_key, &desc_size, &desc_ver);
    mmap_size += 8*desc_size;
    void *mmap = NULL;
    BS->AllocatePool(EfiLoaderData, mmap_size, &mmap);
    BS->GetMemoryMap(&mmap_size, (EFI_MEMORY_DESCRIPTOR*)mmap, &map_key, &desc_size, &desc_ver);

    boot.mmap = (uint64_t)(uintptr_t)mmap;
    boot.mmap_size = mmap_size;
    boot.mmap_desc_size = desc_size;
    boot.mmap_desc_version = desc_ver;

    EFI_STATUS st3 = BS->ExitBootServices(ImageHandle, map_key);
    if (EFI_ERROR(st3)) {
        BS->GetMemoryMap(&mmap_size, (EFI_MEMORY_DESCRIPTOR*)mmap, &map_key, &desc_size, &desc_ver);
        st3 = BS->ExitBootServices(ImageHandle, map_key);
        if (EFI_ERROR(st3)) { for(;;){} }
    }

    // --- Switch CR3 ---
    asm volatile ("mov %0, %%cr3" :: "r"(pml4_phys) : "memory");

    // --- Jump to kernel ---
    typedef void (*kentry_t)(boot_info_t*);
    kentry_t entry = (kentry_t)(uintptr_t)KERNEL_VMA;
    uint64_t aligned_stack_top = stack_top & ~0xFULL;

    asm volatile(
        "mov %[sp], %%rsp\n\t"
        "mov %[b], %%rdi\n\t"
        "jmp *%[e]\n\t"
        :
        : [sp]"r"(aligned_stack_top), [e]"r"(entry), [b]"r"(&boot)
        : "rax","rcx","rdx","rsi","r8","r9","r10","r11","memory"
    );

    for(;;) { __asm__ volatile("hlt"); }
}

