#include <vm.h>
#include <mm.h>
#include <proc.h>
#include <printk.h>
#include <stdlib.h>
#include <string.h>
#include <private_kdefs.h>
#include <csr.h>

static struct task_struct *task[NR_TASKS]; // 线程数组，所有的线程都保存在此
static struct task_struct *idle;           // idle 线程
struct task_struct *current;               // 当前运行线程

extern uint64_t swapper_pg_dir[PGSIZE / 8] __attribute__((__aligned__(PGSIZE)));
extern uint8_t _suapp[];
extern uint8_t _euapp[];

void __dummy(void);
void __switch_to(struct task_struct *prev, struct task_struct *next);

int randint(int low, int high) {
  if (low == high) {
    return low;
  } else {
    return rand() % (high - low + 1) + low;
  }
}

int get_max_cnt_pid(void) {
    int max_cnt_pid = 0;
    for (uint64_t i = 1; i < NR_TASKS; i++) {
        if (task[i]->counter > task[max_cnt_pid]->counter) {
            max_cnt_pid = i;
        }
    }
    return max_cnt_pid;
}

void task_init(void) {
    srand(2025);

    idle = alloc_page();
    idle->pid = 0;
    idle->state = TASK_RUNNING;
    idle->priority = 0;
    idle->counter = 0;

    current = idle;
    task[0] = idle;
    
    for (uint64_t i = 1; i < NR_TASKS; i++) {
        task[i] = alloc_page();
        task[i]->pid = i;
        task[i]->state = TASK_RUNNING;
        task[i]->priority = randint(PRIORITY_MIN, PRIORITY_MAX);
        task[i]->counter = 0;

        task[i]->thread.ra = (uint64_t)&__dummy;
        task[i]->thread.sp = (uint64_t)task[i] + PGSIZE;
        
        task[i]->thread.sepc = USER_START;
        csr_set_bit(task[i]->thread.sstatus, CSR_SSTATUS_SPP, 0);  // set spp = 0
        csr_set_bit(task[i]->thread.sstatus, CSR_SSTATUS_SUM, 1);  // set sum = 1

        task[i]->thread.sscratch = USER_END;
        task[i]->thread.stval = 0;
        task[i]->thread.scause = 0;

        // set per-thread page table
        task[i]->pgd = (pagetable_t)alloc_page();   // VA
        memcpy(task[i]->pgd, swapper_pg_dir, PGSIZE);

        // set user stack
        // uint64_t* user_stack = (uint64_t*)alloc_page();
        // create_mapping(task[i]->pgd, (uint64_t*)(USER_END - PGSIZE), (uint64_t*)VA2PA(user_stack), PGSIZE, SV39_PTE_R | SV39_PTE_W | SV39_PTE_U);

        // set user program
        // uint64_t uapp_begin_page = (uint64_t)_suapp >> PAGE_SHIFT;
        // uint64_t uapp_end_page = ((uint64_t)_euapp - 1) >> PAGE_SHIFT;
        // uint64_t uapp_pages = uapp_end_page - uapp_begin_page + 1;
        // uint64_t* uapp = (uint64_t*)alloc_pages(uapp_pages);
        // memcpy(uapp, (uint64_t*)(uapp_begin_page << PAGE_SHIFT), uapp_pages << PAGE_SHIFT);
        // create_mapping(task[i]->pgd, (uint64_t*)USER_START, (uint64_t*)VA2PA(uapp), uapp_pages << PAGE_SHIFT, SV39_PTE_R | SV39_PTE_W | SV39_PTE_X | SV39_PTE_U);

        // Create VMA for user program
        struct mm_struct *mm = (struct mm_struct*)alloc_page();
        mm->mmap = NULL;
        mm->num_vmas = 0;

        do_mmap(mm, (void*)USER_START, (size_t)(_euapp - _suapp), VM_READ | VM_WRITE | VM_EXEC);
        do_mmap(mm, (void*)(USER_END - PGSIZE), (size_t)PGSIZE, VM_READ | VM_WRITE | VM_ANON);
        
        task[i]->mm = mm;
    }

    printk("...task_init done!\n");
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
            printk("[PID = %" PRIu64 " @ 0x%" PRIx64 "] Running. local = %" PRIu32 ", counter = %" PRIu64 "\n", current->pid, current->thread.sp, ++local, current->counter);
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
        printk("switch to [PID = %" PRIu64 ", PRIORITY = %" PRIu64 ", COUNTER = %" PRIu64 "]\n", current->pid, current->priority, current->counter);
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
        for (uint64_t i = 1; i < NR_TASKS; i++) {
            task[i]->counter = task[i]->priority;

#ifdef ONBOARD
            printk("SET [P=%" PRIu64 "]\n", i);
#else
            printk("SET [PID = %" PRIu64 ", PRIORITY = %" PRIu64 ", COUNTER = %" PRIu64 "]\n", i, task[i]->priority, task[i]->counter);
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

void* do_mmap(struct mm_struct* mm, void* va, size_t len, unsigned flags) {
    struct vm_area_struct* vma;
    if (mm->mmap == NULL) {
        mm->mmap = (struct vm_area_struct*)((unsigned char*)mm + sizeof(struct mm_struct));

        vma = mm->mmap;
        vma->vm_prev = NULL;
        vma->vm_next = NULL;
    } else {
        vma = mm->mmap + mm->num_vmas;
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

    mm->num_vmas++;

    return va;
}
