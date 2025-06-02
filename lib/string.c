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

int memcmp(const void *restrict s1, const void *restrict s2, size_t n) {
    const unsigned char *p1 = s1;
    const unsigned char *p2 = s2;
    for (size_t i = 0; i < n; i++) {
        if (*p1 != *p2) {
            return (*p1 < *p2) ? -1 : 1;
        }
        p1++;
        p2++;
    }
    return 0;
}

void strcpy(char *restrict dst, const char *restrict src) {
    while ((*dst++ = *src++) != '\0') ;
}

void strncpy(char *restrict dst, const char *restrict src, size_t n) {
    size_t i = 0;
    while ((*dst++ = *src++) != '\0' && i < n) {
        i++;
    }
}

size_t strlen(const char *restrict s) {
    size_t len = 0;
    while (*(s++) != '\0') {
        len++;
    }
    return len;
}

size_t strnlen(const char *restrict s, size_t maxlen) {
    size_t len = 0;
    while (len < maxlen && s[len] != '\0') {
        len++;
    }
    return len;
}
