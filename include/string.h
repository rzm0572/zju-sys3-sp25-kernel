#ifndef __STRING_H__
#define __STRING_H__

#include <stddef.h>

void *memset(void *restrict dst, int c, size_t n);

void *memcpy(void *restrict dst, const void *restrict src, size_t n);

int memcmp(const void *restrict s1, const void *restrict s2, size_t n);

void strcpy(char *restrict dst, const char *restrict src);

void strncpy(char *restrict dst, const char *restrict src, size_t n);

size_t strlen(const char *restrict s);

size_t strnlen(const char *restrict s, size_t maxlen);

#endif
