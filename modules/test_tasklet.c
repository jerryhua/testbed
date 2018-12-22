#include <asm/msr.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/cpuset.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <linux/interrupt.h>


static void _taskletProcess(unsigned long notUsed);

DECLARE_TASKLET (appexTasklet, _taskletProcess, 0);

static int g_cpuid = 2;

/* Main tasklet */
static void
_taskletProcess(unsigned long notUsed)
{
    unsigned long long start,end;
    int curr_cpuid;
    unsigned int  max = 0xF0000000;
    int i = 0, j = 0;
    int tmp[256];
    
    curr_cpuid =  get_cpu();
    put_cpu();

    start = rdtsc();
    mb();
    while (i < max)
    {
        i++;
        j = j * i;
        tmp[ j % 256 ] = i;
    }
    mb();
    end = rdtsc();    

    printk("[%s:%d]in tasklet: curr_cpuid=%d i=%d, j=%x, start=%llu, end=%llu, time=%llu\n", 
        __FUNCTION__, __LINE__, curr_cpuid, i, j, start, end, end - start);

 //   tasklet_schedule(&appexTasklet);

    return ;
}

void tasklet_sched(void* data)
{
    int curr_cpuid;
    curr_cpuid =  get_cpu();
    put_cpu();
    printk("[%s:%d]in tasklet: curr_cpuid=%d\n", __FUNCTION__, __LINE__, curr_cpuid);

    tasklet_schedule(&appexTasklet);
    return;
}

static int hg_init(void)
{
    int curr_cpuid;

    curr_cpuid =  get_cpu();
    put_cpu();
    printk("[%s:%d] curr_cpuid=%d\n", __FUNCTION__, __LINE__, curr_cpuid);
    
    smp_call_function_single (g_cpuid, tasklet_sched, NULL, true);

    curr_cpuid =  get_cpu();
    put_cpu();
    printk("[%s:%d] after ipi: curr_cpuid=%d\n", __FUNCTION__, __LINE__, curr_cpuid);

    goto __out;
__out:
	return 0;
}

static void hg_exit(void)
{
	printk(KERN_ALERT"appex hello-world modules exit\n");
}


module_init(hg_init);
module_exit(hg_exit);

MODULE_LICENSE("GPL");

