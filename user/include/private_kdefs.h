#ifndef _PRIVATE_KDEFS_H_
#define _PRIVATE_KDEFS_H_

// #define ONBOARD

#ifdef ONBOARD
    #define TIMECLOCK 800000
#else
    #define TIMECLOCK 10000000
#endif

#endif /* _PRIVATE_KDEFS_H_ */