#include <printk.h>
#include <private_kdefs.h>
#include <sbi.h>

_Noreturn void start_kernel(void) {
  printk("a");
  
  printk("2024 ZJU Computer System II\n");

  // 等待第一次时钟中断
  while (1)
    ;
}
