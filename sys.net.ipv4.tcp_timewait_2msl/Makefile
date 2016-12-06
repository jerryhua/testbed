CURRENT := $(shell uname -r)

WARN_OPTS := -Wall -Werror -Wno-strict-prototypes

ifneq ($(KERNELRELEASE),) 
obj-m := tcp_tw.o
else  
KERNELDIR := /lib/modules/$(CURRENT)/build

PWD := $(shell pwd) 
MODFLAGS := -Wall
#MODFLAGS := -Wall -DMODULE -D__KERNEL__ -DLINUX

default: 
	$(MAKE) -C $(KERNELDIR) M=$(PWD) $(MODFLAGS) modules 
endif 
clean: 
	rm -f *.ko *.mod.c *.mod.o *.o *.ko.unsigned .*.cmd 



