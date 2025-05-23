#include <printk.h>
#include <stdint.h>
#include <proc.h>
#include <mm.h>

typedef void (*trap_handler_t)(uint64_t scause, uint64_t sepc);

void clock_set_next_event(void);

void supervisor_timer_interrupt_handler(uint64_t scause, uint64_t sepc) {
    // printk("%s", "[S] Supervisor timer interrupt\n");
    (void)scause;
    (void)sepc;
    clock_set_next_event();   // 先设置下一次时钟中断，因为 do_timer 不一定返回到这里
    do_timer();
}

const struct handler_table {
    unsigned char interrupt;
    unsigned char exception_code;
    // void (*handler)(uint64_t scause, uint64_t sepc);
    trap_handler_t handler;
} handler_table_entry[] = {
    {1, 5, FUNC_PTR_TRANS(supervisor_timer_interrupt_handler, trap_handler_t)}
};

void trap_handler(uint64_t scause, uint64_t sepc) {
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
            handler_table_entry[i].handler(scause, sepc);
            break;
        }
    }
    // printk("[Trap] scause: %" PRIx64 ", sepc: %" PRIx64 "\n", scause, sepc);
}
