#include <printk.h>
#include <stdint.h>
#include <proc.h>
#include <reg.h>
#include <syscalls.h>
#include <ksyscalls.h>

extern struct ksyscall_table syscall_table[];

void clock_set_next_event(void);

void supervisor_timer_interrupt_handler(struct pt_regs *regs, uint64_t stval) {
    printk("%s", "\x1b[43m[S]\x1b[0m Supervisor timer interrupt\n");
    (void)regs;
    (void)stval;
    clock_set_next_event();   // 先设置下一次时钟中断，因为 do_timer 不一定返回到这里
    do_timer();
}

void ecall_from_user_mode_handler(struct pt_regs *regs, uint64_t stval) {
    (void)stval;
    regs->sepc += 4;

    uint64_t syscall_nr = regs->x[RISCV_REG_A7];
    uint64_t argc = syscall_table[syscall_nr].argc;
    uint64_t* handler = (uint64_t*)syscall_table[syscall_nr].syscall_handler;

    switch (argc) {
        case 6: asm volatile ("mv a5, %0" :: "r" (regs->x[RISCV_REG_A5]) : "memory", "a0", "a1", "a2", "a3", "a4", "a5"); __attribute__ ((fallthrough));
        case 5: asm volatile ("mv a4, %0" :: "r" (regs->x[RISCV_REG_A4]) : "memory", "a0", "a1", "a2", "a3", "a4", "a5"); __attribute__ ((fallthrough));
        case 4: asm volatile ("mv a3, %0" :: "r" (regs->x[RISCV_REG_A3]) : "memory", "a0", "a1", "a2", "a3", "a4", "a5"); __attribute__ ((fallthrough));
        case 3: asm volatile ("mv a2, %0" :: "r" (regs->x[RISCV_REG_A2]) : "memory", "a0", "a1", "a2", "a3", "a4", "a5"); __attribute__ ((fallthrough));
        case 2: asm volatile ("mv a1, %0" :: "r" (regs->x[RISCV_REG_A1]) : "memory", "a0", "a1", "a2", "a3", "a4", "a5"); __attribute__ ((fallthrough));
        case 1: asm volatile ("mv a0, %0" :: "r" (regs->x[RISCV_REG_A0]) : "memory", "a0", "a1", "a2", "a3", "a4", "a5"); __attribute__ ((fallthrough));
        default: break;
    }

    asm volatile (
        "jalr %1\n"
        "mv %0, a0\n"
        : "=r" (regs->x[RISCV_REG_A0])
        : "r" (handler)
        : "ra", "a0", "memory"
    );
}

struct handler_table {
    unsigned char interrupt;
    unsigned char exception_code;
    void (*handler)(struct pt_regs *regs, uint64_t stval);
} handler_table_entry[] = {
    {1, 5, supervisor_timer_interrupt_handler},
    {0, 8, ecall_from_user_mode_handler}
};

void trap_handler(struct pt_regs *regs, uint64_t scause, uint64_t stval) {
    // 根据 scause 判断 trap 类型
    // 如果是 Supervisor Timer Interrupt：
    // - 打印输出相关信息
    // - 调用 clock_set_next_event 设置下一次时钟中断
    // 其他类型的 trap 可以直接忽略，推荐打印出来供以后调试
    // #error Not yet implemented
    int handler_table_size = sizeof(handler_table_entry) / sizeof(struct handler_table);
    unsigned char interrupt = (unsigned char)(scause >> 63);
    unsigned char exception_code = (unsigned char)(scause & 0x3f);
    for (int i = 0; i < handler_table_size; i++) {
        if (handler_table_entry[i].interrupt == interrupt && handler_table_entry[i].exception_code == exception_code) {
            handler_table_entry[i].handler(regs, stval);
            break;
        }
    }
    // printk("[Trap] scause: %" PRIx64 ", sepc: %" PRIx64 "\n", scause, sepc);
}
