#ifndef _MATH_H_
#define _MATH_H_

#include <stdint.h>

#define DEF_MIN_FUNC(type)        \
type type##_min(type a, type b) { \
    return a < b ? a : b;         \
}

#define DEF_MAX_FUNC(type)        \
type type##_max(type a, type b) { \
    return a > b ? a : b;         \
}

DEF_MIN_FUNC(uint64_t)
DEF_MAX_FUNC(uint64_t)

#endif /* _MATH_H_ */
