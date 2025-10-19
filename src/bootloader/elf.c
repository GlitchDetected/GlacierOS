#include <elf_types.h>
#include <efi.h>
#include <efilib.h>

/* Private defines -----------------------------------------------------------*/
#define PAGE_SIZE 4096
#define KERNEL_PHYS_START 0x100000

/* Private function prototypes -----------------------------------------------*/
void print_elf_info(UINT8 *buffer);

/* Private functions ---------------------------------------------------------*/
void print_elf_info(UINT8 *buffer)
{
    elf64_header_t *header = (elf64_header_t *)buffer;
    elf64_program_header_t *program_header =
        (elf64_program_header_t *)(buffer + header->e_phoff);

    Print(L"ELF type: %d, machine: %d, entry: 0x%lx\n",
          header->e_type,
          header->e_machine,
          header->e_entry);

    /* Print all segment info. */
    for (INT32 i = 0; i < header->e_phnum; i++, program_header++)
    {
        Print(L"ELF section:%d type: 0x%x, flag: 0x%x, "
              "offset: 0x%lx, paddr: 0x%lx, vaddr: 0x%lx, filesz: 0x%lx, memsz: 0x%lx\n",
              i,
              program_header->p_type,
              program_header->p_flags64,
              program_header->p_offset,
              program_header->p_paddr,
              program_header->p_vaddr,
              program_header->p_filesz,
              program_header->p_memsz);
    }
}

/* Public functions ----------------------------------------------------------*/
EFI_STATUS load_elf_kernel(UINT8 *buffer,
                           UINT64 size,
                           void **entry_point)
{
    EFI_STATUS res = EFI_SUCCESS;
    elf64_header_t *header = (elf64_header_t *)buffer;
    elf64_program_header_t *program_header = NULL;

    if (header->e_ident.ei_magic0 != 0x7F ||
        header->e_ident.ei_magic1 != 'E' ||
        header->e_ident.ei_magic2 != 'L' ||
        header->e_ident.ei_magic3 != 'F')
    {
        Print(L"kernel is not in ELF format.\n");
        return EFI_LOAD_ERROR;
    }

    /* Accept both EXEC and DYN types */
    if (header->e_type != ELF64_E_TYPE_ET_EXEC && 
        header->e_type != ELF64_E_TYPE_ET_DYN)
    {
        Print(L"ELF type is not supported: %d\n", header->e_type);
        return EFI_LOAD_ERROR;
    }

    Print(L"Loading ELF kernel...\n");
    print_elf_info(buffer);

    /* Load each PT_LOAD segment to its physical address */
    program_header = (elf64_program_header_t *)(buffer + header->e_phoff);
    for (INT32 i = 0; i < header->e_phnum; i++, program_header++)
    {
        if (program_header->p_type == ELF64_P_PT_LOAD)
        {
            UINT8 *dst = (UINT8 *)(UINTN)program_header->p_paddr;
            UINT8 *src = (UINT8 *)buffer + program_header->p_offset;
            UINT64 filesz = program_header->p_filesz;
            UINT64 memsz = program_header->p_memsz;

            Print(L"Loading segment %d to paddr: 0x%lx, filesz: 0x%lx, memsz: 0x%lx\n",
                  i, program_header->p_paddr, filesz, memsz);

            /* Copy file content */
            uefi_call_wrapper(CopyMem, 3, dst, src, filesz);

            /* Zero out BSS (memsz > filesz) */
            if (memsz > filesz)
            {
                uefi_call_wrapper(SetMem, 3, dst + filesz, memsz - filesz, 0);
                Print(L"Zeroed BSS: 0x%lx bytes at 0x%lx\n", 
                      memsz - filesz, (UINT64)(dst + filesz));
            }
        }
    }

    /* Set entry point from ELF header */
    *entry_point = (VOID *)(UINTN)header->e_entry;

    Print(L"ELF loaded successfully, entry point: 0x%lx\n", header->e_entry);

    return EFI_SUCCESS;
}