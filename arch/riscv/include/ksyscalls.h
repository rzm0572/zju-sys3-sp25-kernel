#ifndef __KSYSCALLS_H__
#define __KSYSCALLS_H__

#include <stddef.h>
#include <stdint.h>
#include <syscalls.h>
#include <time.h>

struct pt_regs;

long sys_write(unsigned fd, const char *buf, size_t count);
long sys_getpid(void);
long sys_clone(struct pt_regs *regs);
long sys_clock_gettime(clockid_t clock_id, struct timespec *tp);

typedef long (*syscall_handler_t)();

struct ksyscall_table {
    uint64_t argc;
    syscall_handler_t syscall_handler;
};

#endif
