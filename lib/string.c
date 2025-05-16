#include <string.h>

void *memset(void *restrict dst, int c, size_t n) {
    unsigned char *p = dst;
    unsigned char val = (unsigned char)c;
    for (size_t i = 0; i < n; i++) {
        *p++ = val;
    }
    return dst;
}

void *memcpy(void *restrict dst, const void *restrict src, size_t n) {
    unsigned char *p_dst = dst;
    const unsigned char *p_src = src;
    for (size_t i = 0; i < n; i++) {
        *p_dst++ = *p_src++;
    }
    return dst;
}

size_t strnlen(const char *restrict s, size_t maxlen) {
    size_t len = 0;
    while (len < maxlen && s[len] != '\0') {
        len++;
    }
    return len;
}
