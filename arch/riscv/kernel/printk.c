#include "mm.h"
#include <printk.h>
#include <sbi.h>
#include <stdio.h>

static size_t printk_sbi_write(FILE *restrict fp, const void *restrict buf, size_t len) {
    (void)fp;

    // 调用 SBI 接口输出 buf 中长度为 len 的内容
    // 返回实际输出的字节数
    // Hint：阅读 SBI v2.0 规范！
    struct sbiret ret = sbi_ecall(EID(DEBUG_CONSOLE), FID(DEBUG_CONSOLE, WRITE), (unsigned long)len, (unsigned long)VA2PA(buf), 0, 0, 0, 0);
    return ret.value;
}

void printk(const char *fmt, ...) {
    FILE printk_out = {
        .write = printk_sbi_write,
    };

    va_list ap;
    va_start(ap, fmt);
    vfprintf(&printk_out, fmt, ap);
    va_end(ap);
}
