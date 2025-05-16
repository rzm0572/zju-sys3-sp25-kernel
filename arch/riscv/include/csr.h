#ifndef __CSR_H__
#define __CSR_H__

#define csr_read(csr)                                            \
    ({                                                           \
        uint64_t __v;                                            \
        asm volatile("csrr %0, " #csr : "=r"(__v) : : "memory"); \
        __v;                                                     \
    })

#define csr_write(csr, val)                                        \
    ({                                                             \
        uint64_t __v = (uint64_t)(val);                            \
        asm volatile("csrw " #csr ", %0" : : "r"(__v) : "memory"); \
    })


#define csr_modify(csr, mask, val) (((csr) & ~(mask)) | ((val) & (mask)))

#define csr_set_bit(csr, bit, val)                              \
    ({                                                          \
        csr = csr_modify((csr), 1ULL << (bit), (val) << (bit)); \
    })


#define CSR_SSTATUS_SIE 1
#define CSR_SSTATUS_SPIE 5
#define CSR_SSTATUS_SPP 8
#define CSR_SSTATUS_SUM 18

#endif
