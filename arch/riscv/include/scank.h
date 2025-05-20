#ifndef __SCANK_H__
#define __SCANK_H__

#include <inttypes.h>

void scank(const char *fmt, ...) __attribute__((format(scanf, 1, 2)));

#endif 