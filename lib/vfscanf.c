#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>

#define NL_ARGMAX 9

union arg {
  uintmax_t i;
  // long double f;
  void *p;
};

static int in(FILE *f, char *s, size_t l) {
    if (f->read == NULL) {
        stdout->write(f, "No read function\n", 17);
        return -1;
    }
    return f->read(f, s, l);
}

int scanf_core(FILE *f, const char *fmt, va_list *ap, union arg *nl_arg, int *nl_type) {
    (void)nl_arg;
    (void)nl_type;
    
    int matched = 0;
    while (*fmt) {
        while (isspace(*fmt)) {
            fmt++;
        }
        if (*fmt != '%') {
            stdout->write(stdout, "Wrong format\n", 13);
            return -1;
        }
        fmt++;
        if (*fmt == 'd') {
            int *arg = va_arg(*ap, int *);
            char ch;
            int num = 0, sign = 1, res, flag = 0;
            for (;;) {
                res = in(f, &ch, 1);
                if (res <= 0) {
                    stdout->write(stdout, "EOF\n", 4);
                    return -1;
                }
                else {
                    stdout->write(stdout, &ch, 1);
                }
                if (!isdigit(ch)) {
                    if (ch == '-' && !num && !flag) {
                        flag = 1;
                        sign = -1;
                        continue;
                    }
                    else if (ch == '+' && !num && !flag) {
                        flag = 1;
                        continue;
                    }
                    else if (ch == '\n' || ch == '\r' || ch == ' ' || ch == '\b' || ch == 0x7f || ch == 0) {
                        break;
                    }
                    else {
                        stdout->write(stdout, &ch, 1);
                        stdout->write(stdout, "Not a digit\n", 12);
                        return -1;
                        break;
                    }
                }
                num = num * 10 + (ch - '0');
            }
            matched ++;
            *arg = num * sign;
        }
        else if (*fmt == 'c') {
            char *arg = va_arg(*ap, char *);
            char ch;
            int res = in(f, &ch, 1);
            if (res <= 0) {
                stdout->write(stdout, "EOF\n", 4);
                return -1;
            }
            else {
                stdout->write(stdout, &ch, 1);
            }
            matched ++;
            *arg = ch;
        }
        else if (*fmt == 's') {
            char ch;
            char *p = va_arg(*ap, char *);
            int res;
            for (;;) {
                res = in(f, &ch, 1);
                if (res <= 0) {
                    stdout->write(stdout, "EOF\n", 4);
                    return -1;
                }
                else {
                    stdout->write(stdout, &ch, 1);
                }
                if (ch == '\n' || ch == '\r' || ch == ' ' || ch == '\b' || ch == 0x7f || ch == 0) {
                    break;
                }
                *p = ch;
                p++;
            }
            *p = '\0';
            matched ++;
        }
        fmt++;
    }
    return matched;
}
int vfscanf(FILE *restrict f, const char *restrict fmt, va_list ap) {
    int nl_type[NL_ARGMAX + 1] = {0};
    union arg nl_arg[NL_ARGMAX + 1];
    return scanf_core(f, fmt, &ap, nl_arg, nl_type);
}