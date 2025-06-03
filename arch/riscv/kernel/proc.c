#include "fcntl.h"
#include "ksyscalls.h"
#include "mman.h"
#include "signal.h"
#include <reg.h>
#include <vm.h>
#include <mm.h>
#include <proc.h>
#include <printk.h>
#include <stdlib.h>
#include <string.h>
#include <private_kdefs.h>
#include <csr.h>
#include <elf.h>
#include <fs.h>

struct task_struct *task[NR_TASKS];        // 线程数组，所有的线程都保存在此
struct wait_task wait_queue[NR_TASKS];    // 等待队列
static struct task_struct *idle;           // idle 线程
struct task_struct *current;               // 当前运行线程

uint64_t num_tasks;

extern uint64_t swapper_pg_dir[PGSIZE / 8] __attribute__((__aligned__(PGSIZE)));
extern uint8_t _suapp[];
extern uint8_t _euapp[];

void __dummy(void);
void __switch_to(struct task_struct *prev, struct task_struct *next);
void ret_from_fork(void);
void ret_from_execve(struct task_struct *);

int randint(int low, int high) {
  if (low == high) {
    return low;
  } else {
    return rand() % (high - low + 1) + low;
  }
}

int get_max_cnt_pid(void) {
    int max_cnt_pid = 0;
    for (uint64_t i = 1; i < num_tasks; i++) {
        if (task[i] == NULL) {
            continue;
        }
        if (task[i]->state != TASK_RUNNING) {
            continue;
        }
        if (task[i]->counter > task[max_cnt_pid]->counter) {
            max_cnt_pid = i;
        }
    }
    return max_cnt_pid;
}

int get_avail_task_id(void) {
    for (uint64_t i = 1; i < num_tasks; i++) {
        if (task[i] == NULL) {
            return i;
        }
    }
    return num_tasks++;
}

int check_elf_header(void* start_addr) {
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)start_addr;
    if (ehdr->e_ident[EI_MAG0] != ELFMAG0 || ehdr->e_ident[EI_MAG1] != ELFMAG1 || ehdr->e_ident[EI_MAG2] != ELFMAG2 || ehdr->e_ident[EI_MAG3] != ELFMAG3) {
        return -1;
    } else {
        return 0;
    }
}

void load_program(struct task_struct *task, void* start_addr, struct file* file) {
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)start_addr;
    Elf64_Phdr *phdrs = (Elf64_Phdr *)(start_addr + ehdr->e_phoff);
    for (int i = 0; i < ehdr->e_phnum; i++) {
        Elf64_Phdr *phdr = phdrs + i;
        if (phdr->p_type == PT_LOAD) {
            uint64_t start_va = PGROUNDDOWN(phdr->p_vaddr);
            uint64_t end_va = PGROUNDUP(phdr->p_vaddr + phdr->p_memsz);
            unsigned int flags = 0;
            if (phdr->p_flags & PF_R) {
                flags |= VM_READ;
            }
            if (phdr->p_flags & PF_W) {
                flags |= VM_WRITE;
            }
            if (phdr->p_flags & PF_X) {
                flags |= VM_EXEC;
            }
            do_mmap(task->mm, (void *)start_va, (size_t)(end_va - start_va), flags, file, phdr->p_offset, phdr->p_filesz);
        }
    }
    task->thread.sepc = ehdr->e_entry;
}

void task_init(void) {
    srand(2025);

    idle = alloc_page();
    idle->pid = 0;
    idle->fpid = 0;
    idle->state = TASK_RUNNING;
    idle->priority = 0;
    idle->counter = 0;
    idle->cpid = alloc_page();
    memset(idle->cpid, 0, PGSIZE);
    idle->child_cnt = 0;
    wait_queue[0].valid = 0;

    current = idle;
    task[0] = idle;

    num_tasks = 2;
    
    for (uint64_t i = 1; i < num_tasks; i++) {
        task[i] = alloc_page();
        task[i]->pid = i;
        task[i]->fpid = 0;
        task[i]->state = TASK_RUNNING;
        task[i]->priority = randint(PRIORITY_MIN, PRIORITY_MAX);
        task[i]->counter = 0;
        task[i]->cpid = alloc_page();
        memset(idle->cpid, 0, PGSIZE);
        task[i]->child_cnt = 0;
        idle->cpid[idle->child_cnt++] = i;

        signal_init(i);

        task[i]->thread.ra = (uint64_t)&__dummy;
        task[i]->thread.sp = (uint64_t)task[i] + PGSIZE;
        
        csr_set_bit(task[i]->thread.sstatus, CSR_SSTATUS_SPP, 0);  // set spp = 0
        csr_set_bit(task[i]->thread.sstatus, CSR_SSTATUS_SUM, 1);  // set sum = 1

        task[i]->thread.sscratch = USER_END;
        task[i]->thread.stval = 0;
        task[i]->thread.scause = 0;

        // set per-thread page table
        task[i]->pgd = (pagetable_t)alloc_page();   // VA
        copy_mapping(task[i]->pgd, swapper_pg_dir);

        // Create VMA for user program
        task[i]->mm = (struct mm_struct*)alloc_page();
        struct mm_struct *mm = task[i]->mm;
        mm->mmap = NULL;
        mm->num_vmas = 0;
        mm->free_vma_list = NULL;

        Elf64_Ehdr *ehdr = (Elf64_Ehdr*)_suapp;
        if (ehdr->e_ident[EI_MAG0] != ELFMAG0 || ehdr->e_ident[EI_MAG1] != ELFMAG1 || ehdr->e_ident[EI_MAG2] != ELFMAG2 || ehdr->e_ident[EI_MAG3] != ELFMAG3) {
            task[i]->thread.sepc = USER_START;
            do_mmap(mm, (void*)USER_START, (size_t)(_euapp - _suapp), VM_READ | VM_WRITE | VM_EXEC, NULL, 0, (uint64_t)(_euapp - _suapp));
        }
        else {
            load_program(task[i], _suapp, NULL);
        }
        do_mmap(mm, (void*)(USER_END - PGSIZE), (size_t)PGSIZE, VM_READ | VM_WRITE | VM_ANON, NULL, 0, 0);
        task[i]->files = file_init();
    }

    for (uint64_t i = num_tasks; i < NR_TASKS; i++) {
        task[i] = NULL;
    }

    printk(MSG("process", "task_init done!\n"));
}

void dummy_task(void) {
    unsigned local = 0;
    unsigned prev_cnt = 0;
    while (1) {
        if (current->counter != prev_cnt) {
            if (current->counter == 1) {
                current->counter = 0;
            }
            prev_cnt = current->counter;

#ifdef ONBOARD
            printk("[P=%" PRIu64 "] %" PRIu64 "\n", current->pid, ++local);
#else
            printk(MSG("process", "[PID = %" PRIu64 " @ 0x%" PRIx64 "] Running. local = %" PRIu32 ", counter = %" PRIu64 "\n"), current->pid, current->thread.sp, ++local, current->counter);
#endif
        }
    }
}

void switch_to(struct task_struct* next) {
    if (current->pid != next->pid) {
        struct task_struct* curr = current;
        current = next;

#ifdef ONBOARD
        printk("-> [P=%" PRIu64 "]\n", current->pid);
#else
        printk(MSG("process", "switch to [PID = %" PRIu64 ", PRIORITY = %" PRIu64 ", COUNTER = %" PRIu64 "]\n"), current->pid, current->priority, current->counter);
#endif

        __switch_to(curr, next);
    }
}

void do_timer(void) {
    if (current->counter == 0) {
        schedule();
    } else {
        current->counter--;
        if (current->counter == 0) {
            schedule();
        }
    }
}

void schedule(void) {
    int max_cnt_pid = get_max_cnt_pid();
    if (max_cnt_pid == 0) {
        for (uint64_t i = 1; i < num_tasks; i++) {
            if (task[i] == NULL) {
                continue;
            }

            task[i]->counter = task[i]->priority;

#ifdef ONBOARD
            printk("SET [P=%" PRIu64 "]\n", i);
#else
            ;// printk("SET [PID = %" PRIu64 ", PRIORITY = %" PRIu64 ", COUNTER = %" PRIu64 "]\n", i, task[i]->priority, task[i]->counter);
#endif      
        }
        max_cnt_pid = get_max_cnt_pid();
    }
    switch_to(task[max_cnt_pid]);
}

struct vm_area_struct* find_vma(struct mm_struct* mm, void* va) {
    struct vm_area_struct* vma = mm->mmap;
    while (vma != NULL) {
        if (va < vma->vm_start) {
            return NULL;
        } else if (va < vma->vm_end) {
            return vma;
        }
        vma = vma->vm_next;
    }
    return NULL;
}

void* do_mmap(struct mm_struct* mm, void* va, size_t len, unsigned flags, struct file* file, uint64_t pgoff, uint64_t filesz) {
    struct vm_area_struct* vma;
    if (mm->mmap == NULL) {
        mm->mmap = (struct vm_area_struct*)((unsigned char*)mm + sizeof(struct mm_struct));
        for (uint64_t i = 0; i < NR_VMA_SLOTS - 1; i++) {
            mm->mmap[i].vm_next = mm->mmap + i + 1;
        }
        mm->mmap[NR_VMA_SLOTS - 1].vm_next = NULL;
        mm->free_vma_list = mm->mmap;

        vma = mm->free_vma_list;
        mm->free_vma_list = vma->vm_next;
        vma->vm_prev = NULL;
        vma->vm_next = NULL;
    } else {
        vma = mm->free_vma_list;
        mm->free_vma_list = vma->vm_next;
        vma->vm_next = NULL;

        struct vm_area_struct* curr = mm->mmap;
        struct vm_area_struct* prev = NULL;
        while (curr != NULL) {
            if (va < curr->vm_start) {
                break;
            }
            prev = curr;
            curr = curr->vm_next;
        }

        if (prev == NULL) {
            vma->vm_prev = NULL;
            vma->vm_next = mm->mmap;
            mm->mmap->vm_prev = vma;
            mm->mmap = vma;
        } else {
            vma->vm_next = curr;
            vma->vm_prev = prev;
            prev->vm_next = vma;
            if (curr != NULL) {
                curr->vm_prev = vma;
            }
        }
    }

    vma->vm_mm = mm;
    vma->vm_start = va;
    vma->vm_end = (void*)((unsigned char*)va + len);
    vma->vm_flags = flags;
    vma->vm_file = file;
    vma->vm_pgoff = pgoff;
    vma->vm_filesz = filesz;

    mm->num_vmas++;

    return va;
}

void remove_vma(struct mm_struct* mm, struct vm_area_struct* vma) {
    if (vma == mm->mmap) {
        mm->mmap = vma->vm_next;
    }
    if (vma->vm_prev != NULL) vma->vm_prev->vm_next = vma->vm_next;
    if (vma->vm_next != NULL) vma->vm_next->vm_prev = vma->vm_prev;
    memset(vma, 0, sizeof(struct vm_area_struct));
    vma->vm_next = mm->free_vma_list;
    mm->free_vma_list = vma;
    mm->num_vmas--;
}

int do_munmap(struct mm_struct* mm, void* va, size_t len) {
    struct vm_area_struct* vma = find_vma(mm, va);
    if (vma == NULL) {
        return -1;
    }
    if (((uint64_t)va & (PGSIZE - 1)) || (len & (PGSIZE - 1))) {
        return -1;
    }

    if (vma->vm_start > va) {
        return -1;
    }

    struct vm_area_struct* curr = vma;
    while (curr != NULL && curr->vm_end < va + len) {
        if (curr->vm_next == NULL) {
            return -1;
        }
        if (curr->vm_next->vm_start != curr->vm_end) {
            return -1;
        }
        curr = curr->vm_next;
    }

    if (curr == NULL) {
        return -1;
    }

    remove_mapping(current->pgd, va, len);

    struct vm_area_struct* end_vma = curr->vm_next;
    while (vma != end_vma) {
        struct vm_area_struct removed_vma;
        memcpy(&removed_vma, vma, sizeof(struct vm_area_struct));
        remove_vma(mm, vma);
        
        // Warning: Only support ANON vma for now
        if (removed_vma.vm_start < va) {
            do_mmap(mm, removed_vma.vm_start, va - removed_vma.vm_start, removed_vma.vm_flags, removed_vma.vm_file, removed_vma.vm_pgoff, removed_vma.vm_filesz);
        }
        if (removed_vma.vm_end > va + len) {
            do_mmap(mm, va + len, removed_vma.vm_end - (va + len), removed_vma.vm_flags, removed_vma.vm_file, removed_vma.vm_pgoff, removed_vma.vm_filesz);
        }
        vma = removed_vma.vm_next;
    }
    
    return 1;
}

void* copy_mm(struct mm_struct* dst, const struct mm_struct* src) {
    if (src->mmap == NULL) {
        dst->mmap = NULL;
        dst->num_vmas = 0;
        return dst;
    }

    uint64_t delta = (uint64_t)dst - (uint64_t)src;
    
    dst->mmap = (struct vm_area_struct*)((uint64_t)src->mmap + delta);
    dst->num_vmas = src->num_vmas;

    struct vm_area_struct* src_vma = src->mmap;
    struct vm_area_struct* dst_vma = dst->mmap;
    while (src_vma != NULL) {
        memcpy(dst_vma, src_vma, sizeof(struct vm_area_struct));
        dst_vma->vm_mm = dst;
        dst_vma->vm_prev = src_vma->vm_prev == NULL ? NULL : (struct vm_area_struct*)((uint64_t)src_vma->vm_prev + delta);
        dst_vma->vm_next = src_vma->vm_next == NULL ? NULL : (struct vm_area_struct*)((uint64_t)src_vma->vm_next + delta);
        src_vma = src_vma->vm_next;
        dst_vma = dst_vma->vm_next;
    }
    return dst;
}

long do_fork(struct pt_regs* regs) {
    uint64_t child_pid = get_avail_task_id();
    printk(MSG("process", "do_fork: %" PRIu64 " -> %" PRIu64 "\n"), current->pid, child_pid);
    
    if (task[child_pid] == NULL) {
        task[child_pid] = (struct task_struct*)alloc_page();
    }

    memcpy(task[child_pid], current, PGSIZE);
    task[child_pid]->pid = child_pid;
    task[child_pid]->fpid = current->pid;
    task[child_pid]->state = TASK_RUNNING;
    task[child_pid]->priority = randint(PRIORITY_MIN, PRIORITY_MAX);
    task[child_pid]->counter = 0;
    task[child_pid]->cpid = alloc_page();
    memset(task[child_pid]->cpid, 0, PGSIZE);
    task[child_pid]->child_cnt = 0;
    current->cpid[current->child_cnt++] = child_pid;

    task[child_pid]->files = file_init();
    for (int i = 3; i < MAX_FILE_NUMBER; i++) {
        if (current->files->fd_array[i].opened) {
            int res = -1;
            struct file* child_file = &task[child_pid]->files->fd_array[i];
            res = file_open(child_file, current->files->fd_array[i].path, current->files->fd_array[i].flags);
            if (!res) {
                child_file->opened = 1;
                child_file->lseek(child_file, current->files->fd_array[i].cfo, SEEK_SET);
            }
        }
    }

    signal_init(child_pid);
    task[child_pid]->exit_code = 0;

    uint64_t delta = (uint64_t)task[child_pid] - (uint64_t)current;
    task[child_pid]->thread.ra = (uint64_t)&ret_from_fork;
    task[child_pid]->thread.sp = (uint64_t)regs + delta;
    task[child_pid]->thread.sepc = regs->sepc;
    task[child_pid]->thread.stval = 0;
    task[child_pid]->thread.scause = 0;

    task[child_pid]->mm = (struct mm_struct*)alloc_page();
    copy_mm(task[child_pid]->mm, current->mm);

    task[child_pid]->pgd = (pagetable_t)alloc_page();
    copy_mapping(task[child_pid]->pgd, swapper_pg_dir);
    struct vm_area_struct* vma = current->mm->mmap;
    while (vma != NULL) {
        uint64_t start_page_va = PGROUNDDOWN((uint64_t)vma->vm_start);
        uint64_t end_page_va = PGROUNDUP((uint64_t)vma->vm_end);
        for (uint64_t page_va = start_page_va; page_va < end_page_va; page_va += PGSIZE) {
            uint64_t page_vpn = page_va >> PAGE_SHIFT;
            uint64_t* pte_ptr = walk_page_table_pte(current->pgd, page_vpn);

            if (is_valid_pte(pte_ptr)) {
                uint64_t page_ppn = GET_SUBBITMAP(*pte_ptr, SV39_PTE_PPN_BEGIN, SV39_PTE_PPN_END);
                uint64_t page_pa = page_ppn << PAGE_SHIFT;

                ref_page((void*)PA2VA(page_pa));
                *pte_ptr |= SV39_PTE_S;
                *pte_ptr &= ~SV39_PTE_W;
                create_mapping(task[child_pid]->pgd, (void*)page_va, (void*)page_pa, PGSIZE, SV39_GET_PERM(*pte_ptr));
            }
        }
        vma = vma->vm_next;
    }
    asm volatile("sfence.vma" ::: "memory");

    
    struct pt_regs* child_regs = (struct pt_regs*)((uint64_t)regs + delta);
    child_regs->x[RISCV_REG_A0] = 0;
    return child_pid;
}

unsigned int to_vm_flags(int prot, int flags) {
    unsigned int vm_flags = 0;
    if (prot & PROT_READ) {
        vm_flags |= VM_READ;
    }
    if (prot & PROT_WRITE) {
        vm_flags |= VM_WRITE;
    }
    if (prot & PROT_EXEC) {
        vm_flags |= VM_EXEC;
    }
    if (flags & MAP_ANON) {
        vm_flags |= VM_ANON;
    }
    return vm_flags;
}

int do_execve(const char* pathname, char* const argv[], char* const envp[]) {
    (void)argv;
    (void)envp;

    int fd = (int)sys_openat(AT_FDCWD, pathname, O_RDONLY);
    if (fd < 0) {
        return -1;
    }

    char *elf_header = (char*)alloc_page();
    int ret = sys_read(fd, elf_header, PGSIZE);
    if (ret == -1) {
        return -1;
    }

    if (check_elf_header(elf_header) == -1) {
        sys_close(fd);
        return -1;
    }

    struct vm_area_struct* vma = current->mm->mmap;
    while (vma != NULL) {
        struct vm_area_struct* next = vma->vm_next;
        remove_mapping(current->pgd, vma->vm_start, vma->vm_end - vma->vm_start);
        remove_vma(current->mm, vma);
        vma = next;
    }

    free_pages(current->mm);
    current->mm = (struct mm_struct*)alloc_page();
    current->mm->mmap = NULL;
    current->mm->num_vmas = 0;
    current->mm->free_vma_list = NULL;

    struct file* file = &current->files->fd_array[fd];
    load_program(current, elf_header, file);
    free_pages(elf_header);

    do_mmap(current->mm, (void*)(USER_END - PGSIZE), (size_t)PGSIZE, VM_READ | VM_WRITE | VM_EXEC, NULL, 0, 0);

    csr_set_bit(current->thread.sstatus, CSR_SSTATUS_SPP, 0);  // set spp = 0
    csr_set_bit(current->thread.sstatus, CSR_SSTATUS_SUM, 1);  // set sum = 1
    
    current->thread.ra = (uint64_t)&__dummy;
    current->thread.sp = (uint64_t)current + PGSIZE;
    current->thread.sscratch = USER_END;
    current->thread.stval = 0;
    current->thread.scause = 0;
    memset(current->thread.s, 0, sizeof(current->thread.s));

    asm volatile("sfence.vma" ::: "memory");
    ret_from_execve(current);
    
    return -1;
}

void wake_up_process(int pid) {
    wait_queue[pid].valid = 0;
    wait_queue[pid].mask = 0ULL;
    task[pid]->state = TASK_RUNNING;
}

void do_exit(int status) {
    current->state = TASK_ZOMBIE;
    current->priority = 0;
    current->counter = 0;
    current->exit_code = status & 0xff;

    struct vm_area_struct* vma = current->mm->mmap;
    while (vma != NULL) {
        struct vm_area_struct* next = vma->vm_next;
        remove_mapping(current->pgd, vma->vm_start, vma->vm_end - vma->vm_start);
        remove_vma(current->mm, vma);
        vma = next;
    }
    
    free_pages(current->mm);
    current->mm = NULL;

    asm volatile("sfence.vma" ::: "memory");

    // Adoption
    for (uint64_t i = 0; i < current->child_cnt; i++) {
        task[current->cpid[i]]->fpid = 1;
        task[1]->cpid[task[1]->child_cnt++] = current->cpid[i];
    }
    free_pages(current->cpid);
    current->cpid = NULL;
    current->child_cnt = 0;

    for (int i = 3; i < MAX_FILE_NUMBER; i++) {
        if (current->files->fd_array[i].opened) { 
            sys_close(i);
        }
    }
    free_pages(current->files);
    current->files = NULL;

    free_pages(current->signal);
    current->signal = NULL;

    do_kill(current->fpid, SIGCHLD);

    wait_queue[current->pid].valid = 0;
    if (wait_queue[current->fpid].valid && wait_queue[current->fpid].mask & (1 << current->pid)) {
        wake_up_process(current->fpid);
    }

    schedule();
}

void wait_queue_add(int pid, int wpid) {
    if (wpid == -1) {
        wait_queue[pid].mask = -1;
    } else {
        wait_queue[pid].mask |= 1 << wpid;
    }
}

void do_wait(int pid, int *status, int options) {
    current->state = TASK_INTERRUPTIBLE;
    wait_queue_add(current->pid, pid);
    wait_queue[current->pid].valid = 1;
    schedule();
}
