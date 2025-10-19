.section .text
.global syscall
.type syscall, @function

syscall:
    movq %rdi, %rax      # syscall number
    movq %rsi, %rdi      # arg1
    movq %rdx, %rsi      # arg2
    movq %rcx, %rdx      # arg3
    movq %r8,  %r10      # arg4
    movq %r9,  %r8       # arg5
    syscall
    ret

