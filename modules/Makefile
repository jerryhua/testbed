CURRENT := $(shell uname -r)

WARN_OPTS := -Wall -Werror -Wno-strict-prototypes
KBUILD_CFLAGS += -O0
#CFLAGS := $(CFLAGS) -O0

ifneq ($(KERNELRELEASE),) 
obj-m :=  test_concur.o test_process.o test_softlockup.o test_hook.o test_lock_multicore.o
#obj-m :=  test_tasklet.o
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



