#include <scank.h>
#include <sbi.h>
#include <stddef.h>
#include <stdio.h>
#include <mm.h>
#include <vm.h>
#include <proc.h>
#include <printk.h>

extern struct task_struct* current;
extern FILE __iob[3];

int scank_sbi_read(FILE *restrict fp, void *restrict buf, size_t len) {
    (void)fp;
    unsigned long buf_pa = VA2PA(buf);
    if ((uint64_t)buf < USER_END) {
        buf_pa = (unsigned long)walk_page_table((uint64_t*)current->pgd, (uint64_t*)buf);
    }
    *(int *)buf = -1;
    struct sbiret ret;
    while (*(int *)buf == -1) {
        ret = sbi_ecall(EID(DEBUG_CONSOLE), FID(DEBUG_CONSOLE, READ), (unsigned long)len, buf_pa, 0, 0, 0, 0);
    }
    return (int)ret.value;
}

void scank(const char *fmt, ...) {
    FILE *scank_in = stdin;

    va_list ap;
    va_start(ap, fmt);
    vfscanf(scank_in, fmt, ap);
    va_end(ap);
}
