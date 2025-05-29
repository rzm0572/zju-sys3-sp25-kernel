    .section .text.init
    .globl _start
_start:
    call heap_init
    j main
