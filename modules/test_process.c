#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/inetdevice.h>
#include <linux/rtnetlink.h>   //rntl_lock()
#include <net/net_namespace.h>  //struct net init_net

#include <linux/sched.h>    //wake_up_process()
#include <linux/kthread.h>  //kthread_create(),kthread_run()
#include <linux/err.h>      //IS_ERR(),PTR_ERR()

struct task_struct *task;


static char *net_dev;
module_param(net_dev, charp, 0);
MODULE_PARM_DESC(net_dev, "Max number of bonded devices");

/* list all of the processes */
int ps(void)
{
	struct task_struct *p;

	for_each_process(p)
	{
	    if (p->pid == 10691 || p->pid == 10692 || p->pid == 10721)
    	{	
    	    printk("%s[%d]\n", p->comm, p->pid);
            
        }
	}
	return 0;
}

int thread_fun(void *data)
{
	int dd = *(int*)data;
	while(1)
	{
		printk("now tid %d running \n", dd);
		schedule();
	//	yield();
//		usleep(1000000);		
		mdelay(1000);
	}
}

static int ps_init(void)
{
//	int data[2] = {1, 2};
//	struct net_device *cur_dev = NULL;
//	unsigned newflags = 0;
//	char *na = NULL;

	printk(KERN_ALERT"appex test modules loaded\n");
	ps();
//	kthread_run((void*)thread_fun, (void*)&data[0], "thread1");
//	task = kthread_run((void*)thread_fun, (void*)&data[1], "thread2");
	return -1;
}

static void ps_exit(void)
{
	printk(KERN_ALERT"appex test modules exit\n");
//	kthread_stop(task);
}


module_init(ps_init);
module_exit(ps_exit);
MODULE_LICENSE("GPL");


