#ifndef __UNISTD_H__
#define __UNISTD_H__

#include <stddef.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define AT_FDCWD    -100 
#define O_RDONLY    0x0001
#define O_WRONLY    0x0002
#define O_RDWR      0x0003

#define SEEK_SET    0x0000
#define SEEK_CUR    0x0001
#define SEEK_END    0x0002

typedef int pid_t;
typedef long ssize_t;

long open(const char *pathname, int flags);
long close(int fd);
long lseek(int fd, long offset, int whence);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
pid_t getpid(void);
pid_t fork(void);

#endif
