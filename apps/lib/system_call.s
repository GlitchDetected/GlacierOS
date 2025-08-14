.section .text
.global syscall
.type syscall, @function

syscall:
    movq %rdi, %rax      # Syscall number -> rax
    movq %rsi, %rdi      # shift arg1 -> rdi
    movq %rdx, %rsi      # shift arg2 -> rsi
    movq %rcx, %rdx      # shift arg3 -> rdx
    movq %r8,  %rcx      # shift arg4 -> rcx
    movq %r9,  %r8       # shift arg5 -> r8
    int $0x80            # Do the syscall
    ret
