#include <time.h>
#include <unistd.h>
#include <private_kdefs.h>

clock_t clock(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    clock_t clk = (clock_t)(ts.tv_sec * TIMECLOCK + ts.tv_nsec * TIMECLOCK / 1000000);
    return clk;
}
