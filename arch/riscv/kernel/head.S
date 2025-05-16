#include "private_kdefs.h"

    .section .text.init
    .globl _start
_start:
    # 0. 设置 sp
    la sp, _sbss

    # 1. 初始化内存空间
    call mm_init

    # 2. 初始化线程
    call task_init

    # 3. 将 stvec 设置为 _traps
    la t0, _traps
    # la t0, schedule
    csrw stvec, t0

    # 4. 设置 sie[STIE]
    li t0, 0x20
    csrrs x0, sie, t0

    # 5. 设置第一次时钟中断的时间
    li a0, 0x54494D45
    mv a1, x0

#ifdef ONBOARD
    li a2, TIMECLOCK
#else
    li t0, TIMECLOCK
    rdtime a2
    add a2, a2, t0
#endif

    mv a3, x0
    mv a4, x0
    mv a5, x0
    mv a6, x0
    mv a7, x0
    
    call sbi_ecall

    # 6. 设置 sstatus[SIE]
    csrrsi x0, sstatus, 0x2

    # 7. 跳转到 start_kernel
    j start_kernel

    .section .bss.stack
    .space PGSIZE
