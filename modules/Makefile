CURRENT := $(shell uname -r)

WARN_OPTS := -Wall -Werror -Wno-strict-prototypes

ifneq ($(KERNELRELEASE),) 
obj-m :=  test_hook.o
#obj-m :=  test_lock.o
else  
KERNELDIR := /lib/modules/$(CURRENT)/build

PWD := $(shell pwd) 
#MODFLAGS := -D VERMAGIC_STRING="2.6.32 SMP mod_unload "
MODFLAGS := -Wall
#MODFLAGS := -Wall -DMODULE -D__KERNEL__ -DLINUX

default: 
	$(MAKE) -C $(KERNELDIR) M=$(PWD) $(MODFLAGS) modules 
endif 
clean: 
	rm -f *.ko *.mod.c *.mod.o *.o *.ko.unsigned .*.cmd 



