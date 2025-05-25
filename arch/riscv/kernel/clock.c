#include <private_kdefs.h>
#include <sbi.h>
#include <stdint.h>
#include <time.h>

void clock_set_next_event(void) {
#ifdef ONBOARD
    sbi_ecall(EID(SET_TIMER), FID(SET_TIMER, ), TIMECLOCK, 0, 0, 0, 0, 0);
#else
    uint64_t time;

    // 1. 使用 rdtime 指令读取当前时间
    asm volatile("rdtime %0" : "=r"(time));

    // 2. 计算下一次中断的时间
    uint64_t next = time + TIMECLOCK;

    // 3. 调用 sbi_set_timer 设置下一次时钟中断
    sbi_ecall(EID(SET_TIMER), FID(SET_TIMER, ), next, 0, 0, 0, 0, 0);
#endif
}

clock_t get_mclock(void) {
#ifdef ONBOARD
    struct sbiret ret = sbi_ecall(EID(GET_TIMER), FID(GET_TIMER, ), 0, 0, 0, 0, 0, 0);
    return ret.value;
#else
    uint64_t time;
    asm volatile("rdtime %0" : "=r"(time));
    return time;
#endif
}

int do_clock_gettime(clockid_t clock_id, struct timespec *tp) {
    switch (clock_id) {
        case CLOCK_MONOTONIC_RAW: {
            clock_t mclock = get_mclock();
            tp->tv_sec = mclock / TIMECLOCK;
            tp->tv_nsec = (mclock % TIMECLOCK) * 1000000 / TIMECLOCK;
            return 0;
        }
        default: {
            tp->tv_sec = 0;
            tp->tv_nsec = 0;
            return -1;
        }
    }
}

clock_t clock(void) __attribute__((alias("get_mclock")));
