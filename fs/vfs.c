#include <fs.h>
#include <vfs.h>
#include <sbi.h>
#include <inttypes.h>
#include <printk.h>
#include <scank.h>
#include <stdio.h>

char uart_getchar() {
    char ret;
    ret = stdin->read(stdin, &ret, 1);
    return ret;
}

int64_t stdin_read(struct file *file, void *buf, uint64_t len) {
    (void)file;
    // printk("Now reading from stdin: %d\n", len);
    char *to_read = (char *)buf;
    for (uint64_t i = 0; i < len; i++) {
        to_read[i] = uart_getchar();
        // printk("%d\n", i);
        stdout->write(stdout, &to_read[i], 1);
    }
    to_read[len] = 0;
    return len;
}

int64_t stdout_write(struct file *file, const void *buf, uint64_t len) {
    (void)file;
    char to_print[len + 1];
    for (uint64_t i = 0; i < len; i++) {
        to_print[i] = ((const char *)buf)[i];
    }
    to_print[len] = 0;
    return stdout->write(stdout, buf, len);
}

int64_t stderr_write(struct file *file, const void *buf, uint64_t len) {
    (void)file;
    char to_print[len + 1];
    for (uint64_t i = 0; i < len; i++) {
        to_print[i] = ((const char *)buf)[i];
    }
    to_print[len] = 0;
    return stderr->write(stdout, buf, len);
}