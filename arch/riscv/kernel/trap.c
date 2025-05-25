#include <printk.h>
#include <stdint.h>
#include <proc.h>
#include <reg.h>
#include <syscalls.h>
#include <ksyscalls.h>
#include <mm.h>

extern struct ksyscall_table syscall_table[];
typedef void (*trap_handler_t)(struct pt_regs *regs, uint64_t stval);

void clock_set_next_event(void);

void supervisor_timer_interrupt_handler(struct pt_regs *regs, uint64_t stval) {
    // printk("%s", "\x1b[43m[S]\x1b[0m Supervisor timer interrupt\n");
    printk("\x1b[43m[S]\x1b[0m timer interrupt\n");
    (void)regs;
    (void)stval;
    clock_set_next_event();   // 先设置下一次时钟中断，因为 do_timer 不一定返回到这里
    do_timer();
}

void ecall_from_user_mode_handler(struct pt_regs *regs, uint64_t stval) {
    (void)stval;
    regs->sepc += 4;

    uint64_t syscall_nr = regs->x[RISCV_REG_A7];

    switch (syscall_nr) {
        case __NR_write:
            regs->x[RISCV_REG_A0] = sys_write(regs->x[RISCV_REG_A0], (const char*)regs->x[RISCV_REG_A1], regs->x[RISCV_REG_A2]);
            break;
        case __NR_clock_gettime:
            regs->x[RISCV_REG_A0] = sys_clock_gettime(regs->x[RISCV_REG_A0], (struct timespec*)regs->x[RISCV_REG_A1]);
            break;
        case __NR_getpid:
            regs->x[RISCV_REG_A0] = sys_getpid();
            break;
        default:
            regs->x[RISCV_REG_A0] = -1;
            break;
    }
}

const struct handler_table {
    unsigned char interrupt;
    unsigned char exception_code;
    // void (*handler)(uint64_t scause, uint64_t sepc);
    trap_handler_t handler;
} handler_table_entry[] = {
    {1, 5, FUNC_PTR_TRANS(supervisor_timer_interrupt_handler, trap_handler_t)},
    {0, 8, FUNC_PTR_TRANS(ecall_from_user_mode_handler, trap_handler_t)}
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
