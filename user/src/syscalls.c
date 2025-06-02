#include "stdio.h"
#include <syscalls.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

pid_t getpid(void) {
  pid_t ret;
  asm volatile(
    "li a7, %1\n\t"
    "ecall\n\t"
    "mv %0, a0\n\t"
    : "=r"(ret)
    : "i"(__NR_getpid)
    : "a0", "a7", "memory"
  );
  return ret;
}

long open(const char *pathname, int flags) {
  int ret;
  asm volatile(
    "li a7, %1\n\t"
    "mv a0, %2\n\t"
    "mv a1, %3\n\t"
    "ecall\n\t"
    "mv %0, a0\n\t"
    : "=r" (ret)
    : "i"(__NR_open), "r" (pathname), "r" (flags)
    : "a0", "a1", "a7", "memory"
  );
  return ret;
}

long close(int fd) {
  int ret;
  asm volatile(
    "li a7, %1\n\t"
    "mv a0, %2\n\t"
    "ecall\n\t"
    "mv %0, a0\n\t"
    : "=r" (ret)
    : "i"(__NR_close), "r" (fd)
    : "a0", "a7", "memory"
  );
  return ret;
}

long lseek(int fd, long offset, int whence) {
  long ret;
  asm volatile(
    "li a7, %1\n\t"
    "mv a0, %2\n\t"
    "mv a1, %3\n\t"
    "mv a2, %4\n\t"
    "ecall\n\t"
    "mv %0, a0\n\t"
    : "=r" (ret)
    : "i"(__NR_lseek), "r" (fd), "r" (offset), "r" (whence)
    : "a0", "a1", "a2", "a7", "memory"
  );
  return ret;
}

ssize_t read(int fd, void *buf, size_t count) {
  ssize_t ret;
  asm volatile(
    "li a7, %1\n\t"
    "mv a0, %2\n\t"
    "mv a1, %3\n\t"
    "mv a2, %4\n\t"
    "ecall\n\t"
    "mv %0, a0\n\t"
    : "=r" (ret)
    : "i"(__NR_read), "r" (fd), "r" (buf), "r" (count)
    : "a0", "a1", "a2", "a7", "memory"
  );
  return ret;
}

ssize_t write(int fd, const void *buf, size_t count) {
  ssize_t ret;
  asm volatile(
    "li a7, %1\n\t"
    "mv a0, %2\n\t"
    "mv a1, %3\n\t"
    "mv a2, %4\n\t"
    "ecall\n\t"
    "mv %0, a0\n\t"
    : "=r" (ret)
    : "i"(__NR_write), "r" (fd), "r" (buf), "r" (count)
    : "a0", "a1", "a2", "a7", "memory"
  );
  return ret;
}

pid_t fork(void) {
  pid_t ret;
  asm volatile(
    "li a7, %1\n\t"
    "ecall\n\t"
    "mv %0, a0\n\t"
    : "=r"(ret)
    : "i"(__NR_clone)
    : "a0", "a7", "memory"
  );
  return ret;
}

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
  uint64_t ra = 0;
  asm volatile("mv %0, ra" : "=r" (ra));
  printf("ra = %p\n", ra);

  void *ret;
  asm volatile(
    "li a7, %1\n\t"
    "mv a0, %2\n\t"
    "mv a1, %3\n\t"
    "mv a2, %4\n\t"
    "mv a3, %5\n\t"
    "mv a4, %6\n\t"
    "mv a5, %7\n\t"
    "ecall\n\t"
    "mv %0, a0\n\t"
    : "=r" (ret)
    : "i" (__NR_mmap), "r" (addr), "r" (length), "r" (prot), "r" (flags), "r" (fd), "r" (offset)
    : "a0", "a1", "a2", "a3", "a4", "a5", "a7", "memory"
  );
  return ret;
}

int munmap(void *addr, size_t length) {
  long ret;
  asm volatile(
    "li a7, %1\n\t"
    "mv a0, %2\n\t"
    "mv a1, %3\n\t"
    "ecall\n\t"
    "mv %0, a0\n\t"
    : "=r" (ret)
    : "i"(__NR_munmap), "r" (addr), "r" (length)
    : "a0", "a1", "a7", "memory"
  );
  return (int)ret;
}

ssize_t getdents64(int fd, void *dirp, size_t count) {
  ssize_t ret;
  asm volatile(
    "li a7, %1\n\t"
    "mv a0, %2\n\t"
    "mv a1, %3\n\t"
    "mv a2, %4\n\t"
    "ecall\n\t"
    "mv %0, a0\n\t"
    : "=r" (ret)
    : "i"(__NR_getdents64), "r" (fd), "r" (dirp), "r" (count)
    : "a0", "a1", "a2", "a7", "memory"
  );
  return ret;
}
