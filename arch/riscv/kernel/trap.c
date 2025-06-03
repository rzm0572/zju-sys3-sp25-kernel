#include "private_kdefs.h"
#include "sys_dirent.h"
#include <printk.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <proc.h>
#include <reg.h>
#include <syscalls.h>
#include <ksyscalls.h>
#include <mm.h>
#include <vm.h>
#include <math.h>

#define KERNEL_COLOR "\x1b[43m"
#define ERROR_COLOR "\x1b[31m"
#define RESET_COLOR "\x1b[0m"

#define SCAUSE_INST_PAGE_FAULT  0x0000000c
#define SCAUSE_LOAD_PAGE_FAULT  0x0000000d
#define SCAUSE_STORE_PAGE_FAULT 0x0000000f

extern struct ksyscall_table syscall_table[];
extern struct task_struct *current;

extern uint8_t _suapp[];

void clock_set_next_event(void);

void supervisor_timer_interrupt_handler(struct pt_regs *regs, uint64_t scause, uint64_t stval) {
    // printk("%s", KERNEL_COLOR "[S]" RESET_COLOR " Supervisor timer interrupt\n");
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
        case __NR_open: {
            ret = sys_open(
                (const char*)regs->x[RISCV_REG_A0],
                (int)regs->x[RISCV_REG_A1]
            );
            break;
        }
        case __NR_close: {
            ret = sys_close((int)regs->x[RISCV_REG_A0]);
            break;
        }
        case __NR_lseek: {
            ret = sys_lseek(
                (int)regs->x[RISCV_REG_A0],
                (long)regs->x[RISCV_REG_A1],
                (int)regs->x[RISCV_REG_A2]
            );
            break;
        }
        case __NR_read: {
            ret = sys_read(
                (unsigned int)regs->x[RISCV_REG_A0],
                (char*)regs->x[RISCV_REG_A1],
                (size_t)regs->x[RISCV_REG_A2]
            );
            break;
        }
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
        case __NR_clone: {
            ret = sys_clone(regs);
            break;
        }
        case __NR_mmap: {
            ret = sys_mmap(
                (void*)regs->x[RISCV_REG_A0],
                (size_t)regs->x[RISCV_REG_A1],
                (int)regs->x[RISCV_REG_A2],
                (int)regs->x[RISCV_REG_A3],
                (int)regs->x[RISCV_REG_A4],
                (off_t)regs->x[RISCV_REG_A5]
            );
            break;
        }
        case __NR_munmap: {
            ret = sys_munmap(
                (void*)regs->x[RISCV_REG_A0],
                (size_t)regs->x[RISCV_REG_A1]
            );
            break;
        }
        case __NR_getdents64: {
            ret = sys_getdents64(
                (int)regs->x[RISCV_REG_A0],
                (void*)regs->x[RISCV_REG_A1],
                (size_t)regs->x[RISCV_REG_A2]
            );
            break;
        }
        case __NR_execve: {
            ret = sys_execve(
                (const char*)regs->x[RISCV_REG_A0],
                (char* const*)regs->x[RISCV_REG_A1],
                (char* const*)regs->x[RISCV_REG_A2]
            );
            break;
        }
        case __NR_waitpid: {
            ret = sys_waitpid(
                (int)regs->x[RISCV_REG_A0],
                (int*)regs->x[RISCV_REG_A1],
                (int)regs->x[RISCV_REG_A2]
            );
            break;
        }
        case __NR_exit: {
            sys_exit((int)regs->x[RISCV_REG_A0]);
            // break;
        }
        default: {
            printk(ERROR_COLOR "[S] Not implemented syscall: %" PRIu64 RESET_COLOR "\n", syscall_nr);
            break;
        }
    }
    regs->x[RISCV_REG_A0] = ret;
}

uint64_t vma_flags_to_perm(uint64_t flags) {
    // TODO: 现在对 D 位采用的是最简单的处理方式，即认为所有可写的页都是 dirty 页
    //       但是实际上对可写的页的访问不一定是写入操作，因而可写的页不一定是 dirty 页
    //       解决方法是在设置可写的页的映射时，若此次 page fault 的类型不是 Store/AMO，则不设置 D 位
    //       若之后需要写入，由于 D 位没有设置，又会触发 page fault，此时再设置 D 位
    //       缺点是需要两次 page fault
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
        printk(ERR("page fault", "[S] Page fault at invalid address: %p, reason: VMA not found\n"), bad_vaddr);
        return;
    }

    // If the page fault is caused by instruction access, check if the page is executable
    if (scause == SCAUSE_INST_PAGE_FAULT) {
        printk(KERNEL_COLOR "[S]" RESET_COLOR MSG("page fault", "Instruction page fault; sepc = 0x%" PRIx64 ", stval = 0x%" PRIx64 "\n"), regs->sepc, stval);
        if (!(vma->vm_flags & VM_EXEC)) {
            printk(ERROR_COLOR "[S] Instruction page fault at invalid address: 0x%p, reason: not executable" RESET_COLOR "\n", bad_vaddr);
            return;
        }
    }

    // If the page fault is caused by load access, check if the page is readable
    if (scause == SCAUSE_LOAD_PAGE_FAULT) {
        printk(KERNEL_COLOR "[S]" RESET_COLOR MSG("page fault", "Load page fault; sepc = 0x%" PRIx64 ", stval = 0x%" PRIx64 "\n"), regs->sepc, stval);
        if (!(vma->vm_flags & VM_READ)) {
            printk(KERNEL_COLOR "[S]" RESET_COLOR ERR("page fault", "Load page fault at invalid address: 0x%p, reason: not readable\n"), bad_vaddr);
            return;
        }
    }

    // If the page fault is caused by store access, check if the page is writable
    if (scause == SCAUSE_STORE_PAGE_FAULT) {
        printk(KERNEL_COLOR "[S]" RESET_COLOR MSG("page fault", "Store/AMO page fault; sepc = 0x%" PRIx64 ", stval = 0x%" PRIx64 "\n"), regs->sepc, stval);
        if (!(vma->vm_flags & VM_WRITE)) {
            printk(KERNEL_COLOR "[S]" RESET_COLOR ERR("page fault", "Store/AMO page fault at invalid address: 0x%p, reason: not writable\n"), bad_vaddr);
            return;
        }
    }

    // The bad vaddr is valid, allocate a new page and map it to the vma
    void* new_page = alloc_page();
    uint64_t *va_rounddown = (uint64_t*)PGROUNDDOWN((uint64_t)bad_vaddr);
    uint64_t vpn = (uint64_t)va_rounddown >> PAGE_SHIFT;
    uint64_t *pte_ptr = walk_page_table_pte(current->pgd, vpn);

    if (!is_valid_pte(pte_ptr)) {
        if (!(vma->vm_flags & VM_ANON)) {
            int read_len = 0;
            if (vma->vm_file == NULL) {
                void* uapp_page = (_suapp + vma->vm_pgoff) + ((uint64_t)va_rounddown - (uint64_t)vma->vm_start);
                read_len = uint64_t_min(PGSIZE, (uint64_t)vma->vm_end - (uint64_t)va_rounddown);
                memcpy(new_page, uapp_page, read_len);
            } else {
                void* file_buf = new_page;
                vma->vm_file->lseek(vma->vm_file, vma->vm_pgoff + ((uint64_t)va_rounddown - (uint64_t)vma->vm_start), SEEK_SET);
                read_len = vma->vm_file->read(vma->vm_file, file_buf, PGSIZE);
            }
        }
        create_mapping(current->pgd, va_rounddown, (void*)VA2PA(new_page), PGSIZE, vma_flags_to_perm(vma->vm_flags));
        
        asm volatile("sfence.vma" : : : "memory");
    } else {
        uint64_t *pa = (uint64_t*)(GET_SUBBITMAP(*pte_ptr, SV39_PTE_PPN_BEGIN, SV39_PTE_PPN_END) << PAGE_SHIFT);
        if (scause == SCAUSE_STORE_PAGE_FAULT && (*pte_ptr & SV39_PTE_S)) {
            uint64_t ref_cnt = get_ref_cnt(pa);
            if (ref_cnt >= 2) {
                memcpy(new_page, va_rounddown, PGSIZE);
                deref_page((void*)PA2VA(pa));

                uint64_t new_ppn = PHYS2PPN(VA2PA(new_page));
                uint64_t new_perm = vma_flags_to_perm(vma->vm_flags);
                *pte_ptr = SV39_PTE(new_ppn, new_perm);
                asm volatile("sfence.vma" : : : "memory");

                printk(MSG("page fault", "vma = 0x%" PRIx64 ", SHARED PAGE [PID = %" PRIu64 "], copy 0x%" PRIx64 " to 0x%" PRIx64 "\n"), (uint64_t)vma, current->pid, (uint64_t)pa, (uint64_t)VA2PA(new_page));
            } else if (ref_cnt == 1) {
                *pte_ptr &= ~SV39_PTE_S;
                if (vma->vm_flags & VM_WRITE) {
                    *pte_ptr |= SV39_PTE_W;
                }

                asm volatile("sfence.vma" : : : "memory");
            }
        }
    }

    
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
        printk(MSG("trap", "Unhandled trap: scause = %" PRIx64 ", stval = %" PRIx64 "\n"), scause, stval);
    }
}
