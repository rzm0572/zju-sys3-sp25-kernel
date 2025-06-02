#include <stdio.h>
#include <unistd.h>

static int printf_syscall_write(FILE *restrict fp, const void *restrict buf, size_t len) {
  // TODO for you:
  // add fd to the FILE struct and make this look better :)
  // you can just by the way implement the fileno function and use it here
  return (int)write(fileno(fp), buf, len);
}

static int scanf_syscall_read(FILE *restrict fp, void *restrict buf, size_t len) {
  return (int)read(fileno(fp), buf, len);
}

int fileno(FILE *restrict f) {
  return f->fd;
}

int printf(const char *restrict fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int ret = vfprintf(stdout, fmt, ap);
  va_end(ap);
  return ret;
}

FILE __iob[3] = {
    {.fd = STDIN_FILENO, .read = scanf_syscall_read},     // stdin
    {.fd = STDOUT_FILENO, .write = printf_syscall_write}, // stdout
    {.fd = STDERR_FILENO, .write = printf_syscall_write}, // stderr
};
