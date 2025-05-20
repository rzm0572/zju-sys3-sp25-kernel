#ifndef __STDIO_H__
#define __STDIO_H__

#include <stddef.h>
#include <stdarg.h>

typedef struct FILE {
  unsigned fd;
  int (*read)(struct FILE *, void *, size_t);
  int (*write)(struct FILE *, const void *, size_t);
} FILE;

#define stdin (&__iob[0])
#define stdout (&__iob[1])
#define stderr (&__iob[2])

extern FILE __iob[3];

int vfscanf(FILE *restrict f, const char *restrict fmt, va_list ap);

int scanf(FILE *restrict f, const char *restrict fmt, ...);

int vfprintf(FILE *restrict f, const char *restrict fmt, va_list ap);

int printf(const char *restrict fmt, ...);

int fileno(FILE *restrict f);

#endif
