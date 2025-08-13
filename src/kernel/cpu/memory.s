    .section .text
    .global fast_memcpy
    .code64

# fast_memcpy(*dst, *src, size)
# rdi = destination
# rsi = source
# rdx = number of bytes
fast_memcpy:
    mov %rdi, %rax       # save destination pointer for return

    mov %rdx, %rcx       # rcx = total size
    shr $3, %rcx         # rcx = number of 8-byte chunks
    and $7, %edx         # edx = leftover bytes

    rep movsq             # copy 8-byte chunks from [%rsi] -> [%rdi]

    mov %edx, %ecx       # move leftover bytes count to rcx
    rep movsb             # copy remaining bytes 1 by 1

    ret
