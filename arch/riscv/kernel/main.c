#include <scank.h>
#include <printk.h>
#include <private_kdefs.h>
#include <sbi.h>
#include <proc.h>

_Noreturn void start_kernel(void) {
  // char ch;
  // int x, y;
  // char str[100], str2[100];
  // printk("Please input two numbers: \n");
  // scank("%d %d", &x, &y);
  // printk("Input char: \n");
  // scank("%c", &ch);
  // printk("Input a string: \n");
  // scank("%s%s", str, str2);
  // printk("You input: %c\n", ch);
  // printk("You input: %d %d\n", x,y);
  printk("2025 ZJU Computer System III\n");

  // 等待第一次时钟中断
  // while (1)
  //   ;
  schedule();
  
  while (1)
    ;
}
