#include <mm.h>
#include <proc.h>
#include <printk.h>
#include <stdlib.h>
#include <private_kdefs.h>

static struct task_struct *task[NR_TASKS]; // 线程数组，所有的线程都保存在此
static struct task_struct *idle;           // idle 线程
struct task_struct *current;               // 当前运行线程

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
