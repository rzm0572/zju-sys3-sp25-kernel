#ifndef __PRIVATE_KDEFS_H__
#define __PRIVATE_KDEFS_H__

// #define ONBOARD

#define GREENBOLD "\033[1;32m"
#define REDBOLD "\033[1;31m"
#define RESET "\033[0m"
#define MSG(header, msg) GREENBOLD "[" header "]" RESET " " msg
#define ERR(header, msg) REDBOLD "[" header "] " msg RESET

#ifdef ONBOARD
    #define TIMECLOCK 200000
#else
    // QEMU virt 机器的时钟频率为 10 MHz
    #define TIMECLOCK 10000000
#endif

#define PHY_START 0x80000000
#define PHY_SIZE  0x8000000
#define PHY_END   (PHY_START + PHY_SIZE)

#define VM_START 0xffffffe000000000
#define VM_END   0xffffffff00000000
#define VM_SIZE  (VM_END - VM_START)

#define PA2VA_OFFSET (VM_START - PHY_START)

#define USER_START 0x0
#define USER_END   0x4000000000

#define PGSIZE 0x1000
#define PGROUNDDOWN(addr) ((addr) & ~(PGSIZE - 1))
#define PGROUNDUP(addr) PGROUNDDOWN((addr) + PGSIZE - 1)

#define SATP_MODE 0x8000000000000000
#define PAGE_SHIFT 12

#define INVALID_PA (uint64_t*)0xdeadbeef

#endif
