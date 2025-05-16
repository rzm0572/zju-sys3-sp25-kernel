#include "proc.h"
#include <stdio.h>
#include <string.h>
#include <ksyscalls.h>

// #error Not yet implemented

extern struct task_struct *current;

struct ksyscall_table syscall_table[] = {
    [__NR_write] = {3, (syscall_handler_t)sys_write},
    [__NR_getpid] = {0, (syscall_handler_t)sys_getpid}
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
