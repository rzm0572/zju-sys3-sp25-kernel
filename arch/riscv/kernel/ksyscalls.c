#include "proc.h"
#include <stdio.h>
#include <string.h>
#include <ksyscalls.h>
#include <mm.h>

int do_clock_gettime(clockid_t, struct timespec*);

extern struct task_struct *current;
extern uint64_t num_tasks;

const struct ksyscall_table syscall_table[] = {
    [__NR_write] = {3, FUNC_PTR_TRANS(sys_write, syscall_handler_t)},
    [__NR_clock_gettime] = {2, FUNC_PTR_TRANS(sys_clock_gettime, syscall_handler_t)},
    [__NR_getpid] = {0, FUNC_PTR_TRANS(sys_getpid, syscall_handler_t)},
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

// Warning: Only support clock_id == CLOCK_MONOTONIC_RAW now
long sys_clock_gettime(clockid_t clock_id, struct timespec *tp) {
    int ret = do_clock_gettime(clock_id, tp);
    return ret;
}
