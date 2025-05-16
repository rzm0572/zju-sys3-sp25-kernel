#ifndef __PRIVATE_KDEFS_H__
#define __PRIVATE_KDEFS_H__

#define ONBOARD

#ifdef ONBOARD
    #define TIMECLOCK 200000
#else
    // QEMU virt 机器的时钟频率为 10 MHz
    #define TIMECLOCK 10000000
#endif

#define PHY_START 0x80000000
#define PHY_SIZE 0x400000
#define PHY_END (PHY_START + PHY_SIZE)

#define PGSIZE 0x1000
#define PGROUNDDOWN(addr) ((addr) & ~(PGSIZE - 1))
#define PGROUNDUP(addr) PGROUNDDOWN((addr) + PGSIZE - 1)

#endif
