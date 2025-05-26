#include <printk.h>
#include <scank.h>
#include <scank.h>
#include <sbi.h>
#include <stddef.h>
#include <stdio.h>
#include <mm.h>
#include <vm.h>
#include <proc.h>

extern struct task_struct* current;
extern int scank_sbi_read(FILE *restrict fp, void *restrict buf, size_t len);
int printk_sbi_write(FILE *restrict fp, const void *restrict buf, size_t len) {
    (void)fp;

    // 调用 SBI 接口输出 buf 中长度为 len 的内容
    // 返回实际输出的字节数
    // Hint：阅读 SBI v2.0 规范！

    unsigned long buf_pa = VA2PA(buf);
    if ((uint64_t)buf < USER_END) {
        buf_pa = (unsigned long)walk_page_table((uint64_t*)current->pgd, (uint64_t*)buf);
    }

    struct sbiret ret = sbi_ecall(EID(DEBUG_CONSOLE), FID(DEBUG_CONSOLE, WRITE), (unsigned long)len, buf_pa, 0, 0, 0, 0);
    return (int)ret.value;
}


FILE __iob[3] = {
    {.fd = 0, .read = scank_sbi_read},
    {.fd = 1, .write = printk_sbi_write},
    {.fd = 2, .write = printk_sbi_write}
};


void printk(const char *fmt, ...) {
    // FILE printk_out = __iob[1];
    FILE* printk_out = stdout;

    va_list ap;
    va_start(ap, fmt);
    vfprintf(printk_out, fmt, ap);
    va_end(ap);
}
