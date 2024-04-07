#include "file.h"
static int open_file(struct inode *,struct file *);
static int close_file(struct inode *,struct file *);
static ssize_t read_file(struct file *,char *,size_t,loff_t *);
static ssize_t write_file(struct file *,const char *,size_t,loff_t *);

static int open_cnt=0,cur_message_size=0,major_number=-1;
static struct class* register_class=NULL;
static struct device* register_device=NULL;

static struct file_operations file_ops={
  .read=read_file,
  .write=write_file,
  .open=open_file,
  .release=close_file
};
char message[1005];

inline static int open_file(struct inode *node,struct file *file){
  if(open_cnt){
    return -EBUSY;
  }
  open_cnt=1;
  printk(KERN_INFO "file is:%p",file);
  try_module_get(THIS_MODULE);//Avoid removing module when the file is opened.
  return 0;
}
static ssize_t read_file(struct file *file,char *buffer,size_t len,loff_t * offset){
  int ret=copy_to_user(buffer,message,cur_message_size);
  if(ret!=0){
    printk(KERN_ALERT "Error while writing something.%p",file);
    return -EFAULT;
  }
  printk(KERN_INFO "File write successfully %p",file);
  return 0;
}
static ssize_t write_file(struct file *file,const __user char *buffer,size_t len,loff_t * offset){
  printk(KERN_DEBUG "%p,%p",message,message+100000);
//  strncpy(message,buffer,len);
  int ret=copy_from_user(message,buffer,len);
  if(ret){
    printk(KERN_ALERT "Failed to write. %p",file);
    return -EFAULT;
  }
  cur_message_size=strlen(message);
  while(('a'>message[cur_message_size]||message[cur_message_size]>'z')&&cur_message_size>0&&message[cur_message_size]!='.'){
    message[cur_message_size--]='\0';
  }
  printk(KERN_INFO "File write successfully %p ,%lu, %d",file,len,cur_message_size);
  return len;
}
inline static int close_file(struct inode *node,struct file *file){
  if(open_cnt<=0){
    printk(KERN_ALERT "File not open:%p",file);
    return -ENOENT;
  }
  open_cnt--;
  module_put(THIS_MODULE);
  return 0;
}

int init_file(void) {
  printk(KERN_INFO "Start to insert module.");
  major_number=register_chrdev(0,DEVICE_NAME,&file_ops);
  if(major_number<0){
    printk(KERN_ALERT "Error while registering a major number.");
    return major_number;
  }
  printk(KERN_INFO "Register a major number successfully.");
  register_class=class_create(CLASS_NAME);
  if(IS_ERR(register_class)){
    unregister_chrdev(major_number,DEVICE_NAME);
    printk(KERN_ALERT "Error while registering device class.");
    return PTR_ERR(register_class);
  }
  register_device=device_create(register_class,NULL,MKDEV(major_number,0),NULL,DEVICE_NAME);
  if(IS_ERR(register_device)){
    class_unregister(register_class);
    class_destroy(register_class);
    unregister_chrdev(major_number,DEVICE_NAME);
    printk(KERN_ALERT "Error while registering device. ");
    return PTR_ERR(register_device);
  }
  printk(KERN_INFO "Init successfully.");
  return 0;
}
void exit_file(void) {
  device_destroy(register_class,MKDEV(major_number,0));
  class_unregister(register_class);
  class_destroy(register_class);
  unregister_chrdev(major_number,DEVICE_NAME);
  printk(KERN_INFO "Goobye successfully.");
  return ;
}
