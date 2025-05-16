#include "private_kdefs.h"

    .section .text.init
    .globl _start
_start:
    # 0. 设置 sp
    la sp, _sbss

    # 1.a 初始化页表
    call setup_vm
    call relocate

    # 1.b 初始化内存空间
    call mm_init

    # 1.c 设置最终的三级页表映射关系
    call setup_vm_final

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

    .globl relocate
relocate:
    # modify ra and sp from PA to VA
    li t0, PA2VA_OFFSET
    add ra, ra, t0
    add sp, sp, t0

    # set stvec to VA of label 1
    # la a0, 1f
    # add a0, a0, t0
    # csrrw a1, stvec, a0

    # calculate satp
    li t2, SATP_MODE
    la t1, early_pgtbl
    srai t1, t1, PAGE_SHIFT
    add t2, t2, t1

# .align 2
# 1:
    sfence.vma zero, zero
    csrw satp, t2
#     csrw stvec, a1    # restore stvec
    ret

.section .bss.stack
    .space PGSIZE
