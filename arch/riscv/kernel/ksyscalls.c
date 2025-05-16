#include "proc.h"
#include <stdio.h>
#include <string.h>
#include <ksyscalls.h>

extern struct task_struct *current;
extern uint64_t num_tasks;

struct ksyscall_table syscall_table[] = {
    [__NR_write] = {3, (syscall_handler_t)sys_write},
    [__NR_getpid] = {0, (syscall_handler_t)sys_getpid},
    [__NR_clone] = {1, (syscall_handler_t)sys_clone}
};

long sys_write(unsigned fd, const char *buf, size_t count) {
    FILE* out = &__iob[fd];
    if (out->write == NULL) {
        return -1;
    }

    return out->write(out, buf, count);
}

long sys_getpid(void) {
    return current->pid;
}

long sys_clone(struct pt_regs *regs) {
    return do_fork(regs);
}
