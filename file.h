#ifndef FILE_H_FILE
#define FILE_H_FILE

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/device.h>
#define DEVICE_NAME "HideFiles"
#define CLASS_NAME "HideFiles"

int init_file(void);
void exit_file(void);
extern char message[1005];
#endif
