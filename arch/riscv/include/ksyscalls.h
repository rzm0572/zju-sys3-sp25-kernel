#ifndef __KSYSCALLS_H__
#define __KSYSCALLS_H__

#include <stddef.h>
#include <stdint.h>
#include <syscalls.h>

#define AT_FDCWD -100 

struct pt_regs;

long sys_open(const char *filename, int flags);
long sys_close(int fd);
long sys_lseek(int fd, long offset, int whence);
long sys_read(unsigned fd, char *buf, size_t count);
long sys_write(unsigned fd, const char *buf, size_t count);
long sys_getpid(void);
long sys_clone(struct pt_regs *regs);
long sys_mmap(void *addr, size_t len, int prot, int flags, int fd, long offset);

typedef long (*syscall_handler_t)();

struct ksyscall_table {
    uint64_t argc;
    syscall_handler_t syscall_handler;
};

#endif
