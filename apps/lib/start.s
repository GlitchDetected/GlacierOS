.section .text
.code64

.extern main
.extern init_heap
.global start
.global user_heap_start

start:
    # setup stack
    leaq user_stack_top(%rip), %rsp

    # setup heap
    call init_heap

    # jump into user program
    call main
end:
    jmp end             # infinite loop

.section .bss
.align 4
    .skip 4096*4        # reserve stack
user_stack_top:
