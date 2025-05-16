    .section .text.entry
    .align 4
    .globl _traps
_traps:
    # 1. 将寄存器和 sepc 保存到栈上
    addi sp, sp, -248

    sd ra, 232(sp)
    sd gp, 224(sp)
    sd tp, 216(sp)
    sd t0, 208(sp)
    sd t1, 200(sp)
    sd t2, 192(sp)
    sd s0, 184(sp)
    sd s1, 176(sp)
    sd a0, 168(sp)
    sd a1, 160(sp)
    sd a2, 152(sp)
    sd a3, 144(sp)
    sd a4, 136(sp)
    sd a5, 128(sp)
    sd a6, 120(sp)
    sd a7, 112(sp)
    sd s2, 104(sp)
    sd s3, 96(sp)
    sd s4, 88(sp)
    sd s5, 80(sp)
    sd s6, 72(sp)
    sd s7, 64(sp)
    sd s8, 56(sp)
    sd s9, 48(sp)
    sd s10, 40(sp)
    sd s11, 32(sp)
    sd t3, 24(sp)
    sd t4, 16(sp)
    sd t5, 8(sp)
    sd t6, 0(sp)

    csrr t0, sepc
    sd t0, 240(sp)

    # 2. 调用 trap_handler
    csrr a0, scause
    csrr a1, sepc
    call trap_handler

    # 3. 恢复寄存器和 sepc
    
    ld t0, 240(sp)
    csrw sepc, t0

    ld ra, 232(sp)
    ld gp, 224(sp)
    ld tp, 216(sp)
    ld t0, 208(sp)
    ld t1, 200(sp)
    ld t2, 192(sp)
    ld s0, 184(sp)
    ld s1, 176(sp)
    ld a0, 168(sp)
    ld a1, 160(sp)
    ld a2, 152(sp)
    ld a3, 144(sp)
    ld a4, 136(sp)
    ld a5, 128(sp)
    ld a6, 120(sp)
    ld a7, 112(sp)
    ld s2, 104(sp)
    ld s3, 96(sp)
    ld s4, 88(sp)
    ld s5, 80(sp)
    ld s6, 72(sp)
    ld s7, 64(sp)
    ld s8, 56(sp)
    ld s9, 48(sp)
    ld s10, 40(sp)
    ld s11, 32(sp)
    ld t3, 24(sp)
    ld t4, 16(sp)
    ld t5, 8(sp)
    ld t6, 0(sp)
    addi sp, sp, 248

    csrrsi x0, sstatus, 0x2

    # 4. 返回
    sret


    .globl __dummy
__dummy:
    la t0, dummy_task
    csrw sepc, t0
    sret


    .globl __switch_to
__switch_to:
    # 1. 将当前线程的 CPU Context 保存到 task_struct 上
    addi t0, a0, 0x20
    sd ra, 0(t0)
    sd sp, 8(t0)
    sd s0, 16(t0)
    sd s1, 24(t0)
    sd s2, 32(t0)
    sd s3, 40(t0)
    sd s4, 48(t0)
    sd s5, 56(t0)
    sd s6, 64(t0)
    sd s7, 72(t0)
    sd s8, 80(t0)
    sd s9, 88(t0)
    sd s10, 96(t0)
    sd s11, 104(t0)

    # 2. 将目标线程的 CPU Context 从 task_struct 加载到寄存器
    addi t0, a1, 0x20
    ld ra, 0(t0)
    ld sp, 8(t0)
    ld s0, 16(t0)
    ld s1, 24(t0)
    ld s2, 32(t0)
    ld s3, 40(t0)
    ld s4, 48(t0)
    ld s5, 56(t0)
    ld s6, 64(t0)
    ld s7, 72(t0)
    ld s8, 80(t0)
    ld s9, 88(t0)
    ld s10, 96(t0)
    ld s11, 104(t0)

    ret
