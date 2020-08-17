#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>  //kthread_create(),kthread_run()
#include <linux/delay.h>

struct task_struct *task;
static spinlock_t g_lock;


int thread_fun(void *data)
{
	spin_lock(&g_lock);
	while(!kthread_should_stop())
	{
		mdelay(1000);
	}
	spin_unlock(&g_lock);
	return 0;
}

static int ps_init(void)
{
//	int data[2] = {1, 2};
//	struct net_device *cur_dev = NULL;
//	unsigned newflags = 0;
//	char *na = NULL;
	int val;
	
	spin_lock_init(&g_lock);
	printk(KERN_ALERT"appex test modules loaded, sizeof(spinlock_t)=%d\n", sizeof(g_lock));
//	task = kthread_run((void*)thread_fun, (void*)&val, "test_softlockup");
//	set_cpus_allowed_ptr(task, cpumask_of(0));
	return -1;
}

static void ps_exit(void)
{
	printk(KERN_ALERT"appex test modules exit\n");
	kthread_stop(task);
}


module_init(ps_init);
module_exit(ps_exit);
MODULE_LICENSE("GPL");


