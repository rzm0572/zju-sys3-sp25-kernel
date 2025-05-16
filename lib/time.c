#include "time.h"

clock_t clock(void) {
    clock_t ret;
    asm volatile("rdtime %0" : "=r"(ret));
    // return ret / (TIMECLOCK / CLOCKS_PER_SEC); // bug
    return ret;
}
