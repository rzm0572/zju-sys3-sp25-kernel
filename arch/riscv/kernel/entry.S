#include "private_kdefs.h"
    .section .text.entry
    .align 4
    .globl _traps
_traps:
    # 切换栈
    csrrw sp, sscratch, sp
    bne sp, zero, 1f

    csrr sp, sscratch
1:
    # 1. 将寄存器和 sepc 保存到栈上
    addi sp, sp, -0x110
    
    sd x0, 0x00(sp)
    sd ra, 0x08(sp)
    sd gp, 0x18(sp)
    sd tp, 0x20(sp)
    sd t0, 0x28(sp)
    sd t1, 0x30(sp)
    sd t2, 0x38(sp)
    sd s0, 0x40(sp)
    sd s1, 0x48(sp)
    sd a0, 0x50(sp)
    sd a1, 0x58(sp)
    sd a2, 0x60(sp)
    sd a3, 0x68(sp)
    sd a4, 0x70(sp)
    sd a5, 0x78(sp)
    sd a6, 0x80(sp)
    sd a7, 0x88(sp)
    sd s2, 0x90(sp)
    sd s3, 0x98(sp)
    sd s4, 0xA0(sp)
    sd s5, 0xA8(sp)
    sd s6, 0xB0(sp)
    sd s7, 0xB8(sp)
    sd s8, 0xC0(sp)
    sd s9, 0xC8(sp)
    sd s10, 0xD0(sp)
    sd s11, 0xD8(sp)
    sd t3, 0xE0(sp)
    sd t4, 0xE8(sp)
    sd t5, 0xF0(sp)
    sd t6, 0xF8(sp)

    csrr t0, sepc
    sd t0, 0x100(sp)

    # 保存当前用户态栈指针的值，并将 $sstatus 设为 0
    csrr t0, sscratch
    sd t0, 0x108(sp)

    csrw sscratch, x0

    # 2. 调用 trap_handler
    mv a0, sp
    csrr a1, scause
    csrr a2, stval
    call trap_handler

    # 恢复当前用户态栈指针的值
    ld t0, 0x108(sp)
    csrw sscratch, t0

    # 3. 恢复寄存器和 sepc
    
    ld t0, 0x100(sp)
    csrw sepc, t0

    ld ra, 0x08(sp)
    ld gp, 0x18(sp)
    ld tp, 0x20(sp)
    ld t0, 0x28(sp)
    ld t1, 0x30(sp)
    ld t2, 0x38(sp)
    ld s0, 0x40(sp)
    ld s1, 0x48(sp)
    ld a0, 0x50(sp)
    ld a1, 0x58(sp)
    ld a2, 0x60(sp)
    ld a3, 0x68(sp)
    ld a4, 0x70(sp)
    ld a5, 0x78(sp)
    ld a6, 0x80(sp)
    ld a7, 0x88(sp)
    ld s2, 0x90(sp)
    ld s3, 0x98(sp)
    ld s4, 0xA0(sp)
    ld s5, 0xA8(sp)
    ld s6, 0xB0(sp)
    ld s7, 0xB8(sp)
    ld s8, 0xC0(sp)
    ld s9, 0xC8(sp)
    ld s10, 0xD0(sp)
    ld s11, 0xD8(sp)
    ld t3, 0xE0(sp)
    ld t4, 0xE8(sp)
    ld t5, 0xF0(sp)
    ld t6, 0xF8(sp)
    addi sp, sp, 0x110

    # 切换栈
    csrrw sp, sscratch, sp

    # 4. 返回
    sret


    .globl __dummy
__dummy:
    csrrw sp, sscratch, sp
    sret


    .globl __switch_to
__switch_to:
    # 1. 将当前线程的 CPU Context 保存到 task_struct 上
    addi t0, a0, 0x20
    sd ra, 0x00(t0)
    sd sp, 0x08(t0)
    sd s0, 0x10(t0)
    sd s1, 0x18(t0)
    sd s2, 0x20(t0)
    sd s3, 0x28(t0)
    sd s4, 0x30(t0)
    sd s5, 0x38(t0)
    sd s6, 0x40(t0)
    sd s7, 0x48(t0)
    sd s8, 0x50(t0)
    sd s9, 0x58(t0)
    sd s10, 0x60(t0)
    sd s11, 0x68(t0)

    csrr t1, sepc
    sd t1, 0x70(t0)
    csrr t1, sstatus
    sd t1, 0x78(t0)
    csrr t1, sscratch
    sd t1, 0x80(t0)
    csrr t1, stval
    sd t1, 0x88(t0)
    csrr t1, scause
    sd t1, 0x90(t0)

    # 2. 将目标线程的 CPU Context 从 task_struct 加载到寄存器
    addi t0, a1, 0x20
    ld ra, 0x00(t0)
    ld sp, 0x08(t0)
    ld s0, 0x10(t0)
    ld s1, 0x18(t0)
    ld s2, 0x20(t0)
    ld s3, 0x28(t0)
    ld s4, 0x30(t0)
    ld s5, 0x38(t0)
    ld s6, 0x40(t0)
    ld s7, 0x48(t0)
    ld s8, 0x50(t0)
    ld s9, 0x58(t0)
    ld s10, 0x60(t0)
    ld s11, 0x68(t0)

    ld t1, 0x70(t0)
    csrw sepc, t1
    ld t1, 0x78(t0)
    csrw sstatus, t1
    ld t1, 0x80(t0)
    csrw sscratch, t1
    ld t1, 0x88(t0)
    csrw stval, t1
    ld t1, 0x90(t0)
    csrw scause, t1

    # 3. 切换页表
    addi t0, t0, 0x98
    ld t1, 0(t0)
    li t2, PA2VA_OFFSET
    sub t1, t1, t2
    srai t1, t1, PAGE_SHIFT
    li t2, SATP_MODE
    add t1, t1, t2
    csrw satp, t1
    sfence.vma

    ret
