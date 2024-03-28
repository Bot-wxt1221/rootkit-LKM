
extern void my_hook_kill(void);

extern void my_unregister_hook_kill(void);


extern asmlinkage int hook_tar_kill(pid_t pid, int sig);
