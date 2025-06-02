#ifndef __UNISTD_H__
#define __UNISTD_H__

#include <stddef.h>
#include <stdio.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

typedef int pid_t;
typedef long ssize_t;

long open(const char *pathname, int flags);
long close(int fd);
long lseek(int fd, long offset, int whence);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
pid_t getpid(void);
pid_t fork(void);
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t length);
ssize_t getdents64(int fd, void* dirp, size_t count);

#endif
