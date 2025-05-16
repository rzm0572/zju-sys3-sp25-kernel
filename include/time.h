#ifndef __TIME_H__
#define __TIME_H__

#include <stdint.h>

#define TIMECLOCK 10000000
#define CLOCKS_PER_SEC 1000000

typedef uint64_t clock_t;

clock_t clock(void);

#endif
