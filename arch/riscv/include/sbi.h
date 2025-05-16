#ifndef __SBI_H__
#define __SBI_H__

#include <stdint.h>

#define real_cat(a, b) a ## b
#define cat(a, b) real_cat(a, b)

#define EID(SBI_EXT) cat(SBI_, cat(SBI_EXT, _EID))
#define FID(SBI_EXT, SBI_FUNC) cat(SBI_, cat(SBI_EXT, cat(_, cat(SBI_FUNC, _FID))))

#define SBI_DEBUG_CONSOLE_EID 0x4442434E
#define SBI_SET_TIMER_EID 0x54494D45

#define SBI_DEBUG_CONSOLE_WRITE_FID 0
#define SBI_DEBUG_CONSOLE_READ_FID 1
#define SBI_DEBUG_CONSOLE_WRITE_BYTE_FID 2
#define SBI_SET_TIMER__FID 0

#define csr_read(csr)                                        \
  ({                                                         \
    uint64_t __v;                                            \
    asm volatile("csrr %0, " #csr : "=r"(__v) : : "memory"); \
    __v;                                                     \
  })

#define csr_write(csr, val)                                    \
  ({                                                           \
    uint64_t __v = (uint64_t)(val);                            \
    asm volatile("csrw " #csr ", %0" : : "r"(__v) : "memory"); \
  })

struct sbiret {
    uint64_t error;
    uint64_t value;
};

struct sbiret sbi_ecall(uint64_t eid, uint64_t fid,
                        uint64_t arg0, uint64_t arg1, uint64_t arg2,
                        uint64_t arg3, uint64_t arg4, uint64_t arg5);
#endif
