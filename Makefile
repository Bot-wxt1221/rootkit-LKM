obj-m+=lkm.o
lkm-objs =hooks.o rootkit.o process.o module.o file.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
