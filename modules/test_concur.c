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
#include <linux/cpumask.h>
//#include <asm-generic/atomic64.h>
#include <linux/sched.h>    //wake_up_process()
#include <linux/kthread.h>  //kthread_create(),kthread_run()
#include <linux/err.h>      //IS_ERR(),PTR_ERR()

atomic64_t g_test;
int cpu[128];
struct task_struct *thread[128];

void thread_fun(void *data)
{
	int cpu_id = *(int *)data;

	cpu_set(cpu_id, current->cpus_allowed);
	schedule();

	while (!kthread_should_stop())
	{
		atomic64_inc(&g_test);
		schedule();
	}
}

static int ps_init(void)
{
	int i = 1;

	for (i = 0; i < 128; i++)
	{
		cpu[i] = i;
	}

	for (i = 1; i < num_online_cpus(); i++)
	{
		thread[i] = kthread_run((void*)thread_fun, (void*)&cpu[i], "jtest");
	}
	return 0;
}

static void ps_exit(void)
{
	int i;
	for (i = 1; i < num_online_cpus(); i++)
	{
		kthread_stop(thread[i]);
	}
	printk(KERN_ALERT"appex test modules exit\n");
}


module_init(ps_init);
module_exit(ps_exit);
MODULE_LICENSE("GPL");


