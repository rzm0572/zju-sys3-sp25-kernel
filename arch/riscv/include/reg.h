#ifndef __REG_H__
#define __REG_H__

#include <stdint.h>

#define RISCV_REG_RA 1
#define RISCV_REG_SP 2
#define RISCV_REG_A0 10
#define RISCV_REG_A1 11
#define RISCV_REG_A2 12
#define RISCV_REG_A3 13
#define RISCV_REG_A4 14
#define RISCV_REG_A5 15
#define RISCV_REG_A6 16
#define RISCV_REG_A7 17

// 中断处理所需寄存器堆
struct pt_regs {
  uint64_t x[32];
  uint64_t sepc;
  uint64_t sscratch;
};

#endif
