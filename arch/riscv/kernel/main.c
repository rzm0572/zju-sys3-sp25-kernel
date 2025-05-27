#include <printk.h>
#include <private_kdefs.h>
#include <sbi.h>
#include <proc.h>

_Noreturn void start_kernel(void) {
  
  printk("2025 ZJU Computer System III\n");

  // 等待第一次时钟中断
  // while (1)
  //   ;
  schedule();
  
  while (1)
    ;
}
