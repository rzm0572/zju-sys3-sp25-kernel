#include <printk.h>
#include <stdint.h>
#include <string.h>
#include <proc.h>
#include <reg.h>
#include <syscalls.h>
#include <ksyscalls.h>
#include <mm.h>
#include <vm.h>

#define SCAUSE_INST_PAGE_FAULT  0x0000000c
#define SCAUSE_LOAD_PAGE_FAULT  0x0000000d
#define SCAUSE_STORE_PAGE_FAULT 0x0000000f

extern struct ksyscall_table syscall_table[];
extern struct task_struct *current;

extern uint8_t _suapp[];

void clock_set_next_event(void);

void supervisor_timer_interrupt_handler(struct pt_regs *regs, uint64_t scause, uint64_t stval) {
    // printk("%s", "\x1b[43m[S]\x1b[0m Supervisor timer interrupt\n");
    (void)regs;
    (void)scause;
    (void)stval;
    clock_set_next_event();   // 先设置下一次时钟中断，因为 do_timer 不一定返回到这里
    do_timer();
}

void ecall_from_user_mode_handler(struct pt_regs *regs, uint64_t scause, uint64_t stval) {
    (void)scause;
    (void)stval;
    regs->sepc += 4;

    uint64_t syscall_nr = regs->x[RISCV_REG_A7];

    uint64_t ret = 0;
    switch (syscall_nr) {
        case __NR_write: {
            ret = sys_write(
                (unsigned int)regs->x[RISCV_REG_A0],
                (const char*)regs->x[RISCV_REG_A1],
                (size_t)regs->x[RISCV_REG_A2]
            );
            break;
        }
        case __NR_getpid: {
            ret = sys_getpid();
            break;
        }
        default: {
            printk("[S] Not implemented syscall: %" PRIu64 "\n", syscall_nr);
            break;
        }
    }
    regs->x[RISCV_REG_A0] = ret;
}

uint64_t vma_flags_to_perm(uint64_t flags) {
    uint64_t perm = SV39_PTE_V | SV39_PTE_U | SV39_PTE_A;
    if (flags & VM_READ)  perm |= SV39_PTE_R;
    if (flags & VM_WRITE) perm |= SV39_PTE_W | SV39_PTE_D;
    if (flags & VM_EXEC)  perm |= SV39_PTE_X;
    return perm;
}

void do_page_fault(struct pt_regs *regs, uint64_t scause, uint64_t stval) {
    (void)regs;
    uint64_t* bad_vaddr = (uint64_t*)stval;

    // Get the vm area struct that contains the bad address
    struct vm_area_struct* vma = find_vma(current->mm, (void*)bad_vaddr);
    
    // If the bad vaddr is not belong to any vm area, then it's an invalid address
    if (vma == NULL) {
        printk("\x1b[31m[S] Page fault at invalid address: 0x%p, reason: VMA not found\x1b[0m\n", bad_vaddr);
        return;
    }

    // If the page fault is caused by instruction access, check if the page is executable
    if (scause == SCAUSE_INST_PAGE_FAULT && !(vma->vm_flags & VM_EXEC)) {
        printk("\x1b[31m[S] Instruction access fault at address: 0x%p, reason: not executable\x1b[0m\n", bad_vaddr);
        return;
    }

    // If the page fault is caused by load access, check if the page is readable
    if (scause == SCAUSE_LOAD_PAGE_FAULT && !(vma->vm_flags & VM_READ)) {
        printk("\x1b[31m[S] Load access fault at address: 0x%p, reason: not readable\x1b[0m\n", bad_vaddr);
        return;
    }

    // If the page fault is caused by store access, check if the page is writable
    if (scause == SCAUSE_STORE_PAGE_FAULT && !(vma->vm_flags & VM_WRITE)) {
        printk("\x1b[31m[S] Store access fault at address: 0x%p, reason: not writable\x1b[0m\n", bad_vaddr);
        return;
    }

    // The bad vaddr is valid, allocate a new page and map it to the vma
    void* new_page = alloc_page();
    uint64_t va_rounddown = PGROUNDDOWN((uint64_t)bad_vaddr);
    if (!(vma->vm_flags & VM_ANON)) {
        void* uapp_page = _suapp + va_rounddown;
        memcpy(new_page, uapp_page, PGSIZE);
    }

    create_mapping(current->pgd, (void*)va_rounddown, (void*)VA2PA(new_page), PGSIZE, vma_flags_to_perm(vma->vm_flags));
}

struct handler_table {
    unsigned char interrupt;
    unsigned char exception_code;
    void (*handler)(struct pt_regs *regs, uint64_t scause, uint64_t stval);
} handler_table_entry[] = {
    {1, 5, supervisor_timer_interrupt_handler},
    {0, 8, ecall_from_user_mode_handler},
    {0, SCAUSE_INST_PAGE_FAULT, do_page_fault},
    {0, SCAUSE_LOAD_PAGE_FAULT, do_page_fault},
    {0, SCAUSE_STORE_PAGE_FAULT, do_page_fault}
};

void trap_handler(struct pt_regs *regs, uint64_t scause, uint64_t stval) {
    // 根据 scause 判断 trap 类型
    // 如果是 Supervisor Timer Interrupt：
    // - 打印输出相关信息
    // - 调用 clock_set_next_event 设置下一次时钟中断
    // 其他类型的 trap 可以直接忽略，推荐打印出来供以后调试
    int handler_table_size = sizeof(handler_table_entry) / sizeof(struct handler_table);
    unsigned char interrupt = (unsigned char)(scause >> 63);
    unsigned char exception_code = (unsigned char)(scause & 0x3f);
    int handled = 0;
    for (int i = 0; i < handler_table_size; i++) {
        if (handler_table_entry[i].interrupt == interrupt && handler_table_entry[i].exception_code == exception_code) {
            handler_table_entry[i].handler(regs, scause, stval);
            handled = 1;
            break;
        }
    }
    // printk("[Trap] scause: %" PRIx64 ", sepc: %" PRIx64 "\n", scause, sepc);
    if (!handled) {
        // printk("[S] Unhandled trap: scause = %" PRIx64 ", stval = %" PRIx64 "\n", scause, stval);
    }
}
