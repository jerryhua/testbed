#include <asm/msr.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/cpuset.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <linux/interrupt.h>


static int cpu_num;
static int cpu_array[NR_CPUS];
module_param_array(cpu_array, int, &cpu_num, 0444);


int maxbits = 24;
module_param(maxbits, int, 0444);

static void _taskletProcess(unsigned long notUsed);

DECLARE_TASKLET (appexTasklet1, _taskletProcess, (unsigned long)&appexTasklet1);
DECLARE_TASKLET (appexTasklet2, _taskletProcess, (unsigned long)&appexTasklet2);

static unsigned long long  s_max = 0x1;

/* Main tasklet */
static void
_taskletProcess(unsigned long ptasklet)
{
    unsigned long long start, end;
    int start_j, end_j;
    int curr_cpuid;
    int tmp[256];
    static unsigned long long s_i = 0, s_j = 1;
    static int time_j = 0;
    static unsigned long long time = 0;

    if (s_i == 0)
    {
        curr_cpuid = get_cpu();
        put_cpu();
        printk("[%s:%d]tasklet start: curr_cpuid=%d s_max=%x\n", 
            __FUNCTION__, __LINE__, curr_cpuid, s_max);
    }
    
    start = rdtsc();
    start_j = jiffies;
  //  mb();
    while ((s_i < s_max))
    {
        s_i++;
        s_j = s_j * s_i;
        tmp[ s_j % 256 ] = s_i;
        if (s_i % 10000 == 0)
            break;
    }
  //  mb();
    end = rdtsc();  
    end_j = jiffies;

    time += end - start;
    time_j += end_j - start_j;
    
    if (s_i < s_max)
        tasklet_schedule((struct tasklet_struct*)ptasklet);
    else
        printk("[%s:%d]in tasklet: max=%llu i=%llu, j=%llx, cycles=%llu, jiffies=%d, tmp[j%%156]=%d\n", 
        __FUNCTION__, __LINE__, s_max, s_i, s_j, time, time_j, tmp[s_j%256]);

    return ;
}

void tasklet_sched(void* data)
{
    int curr_cpuid;
    curr_cpuid =  get_cpu();
    put_cpu();
    printk("[%s:%d]in tasklet: curr_cpuid=%d\n", __FUNCTION__, __LINE__, curr_cpuid);

    tasklet_schedule((struct tasklet_struct*)data);
    return;
}

static int hg_init(void)
{
    int curr_cpuid;

    s_max = s_max << maxbits;

    curr_cpuid =  get_cpu();
    put_cpu();
    printk("[%s:%d] curr_cpuid=%d\n", __FUNCTION__, __LINE__, curr_cpuid);
    
    smp_call_function_single (cpu_array[0], tasklet_sched, &appexTasklet1, true);
    smp_call_function_single (cpu_array[1], tasklet_sched, &appexTasklet2, true);

    curr_cpuid =  get_cpu();
    put_cpu();
    printk("[%s:%d] after ipi: curr_cpuid=%d NR_CPUS=%d\n", __FUNCTION__, __LINE__, curr_cpuid, NR_CPUS);

    goto __out;
__out:
	return 0;
}

static void hg_exit(void)
{
    tasklet_kill(&appexTasklet1);
    tasklet_kill(&appexTasklet2);
	printk(KERN_ALERT"appex hello-world modules exit\n");
}


module_init(hg_init);
module_exit(hg_exit);

MODULE_LICENSE("GPL");

