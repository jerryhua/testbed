#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/delay.h>

#include <linux/sched.h>   
#include <linux/kthread.h>  
#include <linux/err.h>     
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <net/net_namespace.h>

static DEFINE_PER_CPU(struct task_struct *, t_thread);

struct test_st
{
  int in_cnt[4096];
  int out_cnt[4096];
  int *percpu_cnt;
  int shared_cnt;
};

struct test4_st
{
    atomic_t atomic_cnt;  
    int cpu1_cnt;
};

rwlock_t test5_rw_lock;  

static struct test4_st test4_stats;
static struct test_st test_stats;
static DEFINE_PER_CPU(u64, g_start_jiffies);
static DEFINE_PER_CPU(u64, g_end_jiffies);

#define BASE_CNT 1000000  //one million
static int g_thread_cnt; //change to atomic
int __percpu *g_count;
//

static int max_cnt = 100;
module_param(max_cnt, int, 0644);
MODULE_PARM_DESC(max_cnt, "max count");

static u64 g_max_count;

static _print_time(char * s, u64 start, u64 end)
{
    u64 ms = 0;

    ms = jiffies_to_msecs(end - start);
    printk("%s total time: %llu s,%llu ms\n", s?s:"", ms / MSEC_PER_SEC, ms % MSEC_PER_SEC);
}

void print_time(char * s)
{
    int cpuid;
    u64 s_jiffies = 0, e_jiffies = 0;
    
    for_each_online_cpu(cpuid)
    {
        if (s_jiffies == 0)
            s_jiffies = per_cpu(g_start_jiffies, cpuid);

        if (e_jiffies == 0)
            e_jiffies = per_cpu(g_end_jiffies, cpuid);

        if (s_jiffies > per_cpu(g_start_jiffies, cpuid))
            s_jiffies = per_cpu(g_start_jiffies, cpuid);
        
        if (e_jiffies < per_cpu(g_end_jiffies, cpuid))
            e_jiffies = per_cpu(g_end_jiffies, cpuid);
    }

    _print_time(s, s_jiffies, e_jiffies);
}

/* each cpu access data in one cache line */
static int test1(void *param)
{
    int cpuid;
    u64 count = 0;

    cpuid = get_cpu(); 
    put_cpu();

    if (cpuid == 0)
    {
        /* reserve cpu0 */
        g_thread_cnt--;
        return 0;
    }
    
    per_cpu(g_start_jiffies, cpuid) = jiffies_64;

    while(likely(count < g_max_count))
	{
        test_stats.in_cnt[cpuid] += count;
        test_stats.out_cnt[cpuid] += count;
        count ++;
	}

    per_cpu(g_end_jiffies, cpuid) = jiffies_64;
    printk("[cpu%d]kthread stoped\n", cpuid);

    g_thread_cnt--;
    if (g_thread_cnt == 0)
        print_time("access same cache line");
    return 0;
}

/* each cpu access data in different cache line */
static int test2(void *param)
{
    int cpuid;
    u64 count = 0;

    cpuid = get_cpu(); 
    put_cpu();

    if (cpuid == 0)
    {
        /* reserve cpu0 */
        g_thread_cnt--;
        return 0;
    }
    
    per_cpu(g_start_jiffies, cpuid) = jiffies_64;

    while(likely(count < g_max_count))
	{
        test_stats.in_cnt[cpuid*16] += count;
        test_stats.out_cnt[cpuid*16] += count;
        count ++;
	}

    per_cpu(g_end_jiffies, cpuid) = jiffies_64;
    printk("[cpu%d]kthread stoped\n", cpuid);

    g_thread_cnt--;
    if (g_thread_cnt == 0)
        print_time("access different cache line");
    return 0;
}

/* each cpu access data in percpu data */
static int test3(void *param)
{
    int cpuid;
    u64 count = 0;

    cpuid = get_cpu(); 
    put_cpu();

    if (cpuid == 0)
    {
        /* reserve cpu0 */
        g_thread_cnt--;
        return 0;
    }
    
    per_cpu(g_start_jiffies, cpuid) = jiffies_64;

    while(likely(count < g_max_count))
	{
//        preempt_disable();                           
        *this_cpu_ptr(test_stats.percpu_cnt) += count; 
   //     preempt_enable();   

        count ++;
	}

    per_cpu(g_end_jiffies, cpuid) = jiffies_64;
    printk("[cpu%d]kthread stoped\n", cpuid);

    g_thread_cnt--;
    if (g_thread_cnt == 0)
        print_time("access percpu");
    return 0;
}

static void test4_1(void *param)
{
    u64 local_start_jiffies, local_end_jiffies;
    u64 count = 0;

    local_start_jiffies = jiffies_64;
    while(likely(count < g_max_count))
	{
	    test4_stats.cpu1_cnt += count;
        count ++;
	}
    local_end_jiffies = jiffies_64;

    _print_time("[test4]access local vary(other vary in same cache line changing)", local_start_jiffies, local_end_jiffies); 
   

    while(g_thread_cnt != 1)
        msleep(1);
    printk("[except cpu1]kthread stoped\n");

    count = 0;
    local_start_jiffies = jiffies_64;
    while(likely(count < g_max_count))
	{
	    test4_stats.cpu1_cnt += count;
        count ++;
	}
    local_end_jiffies = jiffies_64;
    
    _print_time("[test4]access local vary(other vary in same cache line not change)", local_start_jiffies, local_end_jiffies); 
}

static int test4(void *param)
{
    int cpuid;
    u64 count = 0;

    cpuid = get_cpu(); 
    put_cpu();

    if (cpuid == 0)
    {
        /* reserve cpu0 */
        g_thread_cnt--;
        atomic_set(&test4_stats.atomic_cnt, 0);
        return 0;
    }

    if (cpuid == 1)
    {
        test4_1(param);
        g_thread_cnt--;
        return 0;
    }
    
    while(likely(count < g_max_count))
	{
        atomic_add(count, &test4_stats.atomic_cnt);
        count ++;
	}

    g_thread_cnt--;
    return 0;
}

static void test5_1(void *param)
{
    u64 local_start_jiffies, local_end_jiffies;
    u64 count = 0;

    while(g_thread_cnt != 1)
        msleep(1);
    printk("[except cpu1]kthread stoped\n");

    local_start_jiffies = jiffies_64;
    while(likely(count < g_max_count))
	{
	    read_lock(&test5_rw_lock);
        count ++;
        read_unlock(&test5_rw_lock);
	}
    local_end_jiffies = jiffies_64;
    
    _print_time("[test5]read_lock only one thread ", local_start_jiffies, local_end_jiffies); 
}

/* test rwlock */
static int test5(void *param)
{
    int cpuid;
    u64 count = 0;
    char buf[128] = {0};

    cpuid = get_cpu(); 
    put_cpu();

    if (cpuid == 0)
    {
        rwlock_init(&test5_rw_lock);
        /* reserve cpu0 */
        g_thread_cnt--;
        return 0;
    }

    if (cpuid == 1)
    {
        test5_1(param);
        g_thread_cnt--;
        return 0;
    }
    
    per_cpu(g_start_jiffies, cpuid) = jiffies_64;

    while(likely(count < g_max_count))
	{
	    read_lock(&test5_rw_lock);
        count ++;
        read_unlock(&test5_rw_lock);
	}

    per_cpu(g_end_jiffies, cpuid) = jiffies_64;
    printk("[cpu%d]kthread stoped\n", cpuid);

    g_thread_cnt--;
    snprintf(buf, sizeof(buf), "[test5]read_lock %d thread", num_online_cpus() - 2);
    if (g_thread_cnt == 1)
        print_time(buf);
    return 0;
}

static void test6_1(void *param)
{
    u64 local_start_jiffies, local_end_jiffies;
    u64 count = 0;

    while(g_thread_cnt != 1)
        msleep(1);
    printk("[except cpu1]kthread stoped\n");

    local_start_jiffies = jiffies_64;
    while(likely(count < g_max_count))
	{
	    rcu_read_lock();
        count ++;
        rcu_read_unlock();
	}
    local_end_jiffies = jiffies_64;
    
    _print_time("[test5]rcu_read_lock only one thread ", local_start_jiffies, local_end_jiffies); 
}

/* test rcu lock */
static int test6(void *param)
{
    int cpuid;
    u64 count = 0;
    char buf[128] = {0};

    cpuid = get_cpu(); 
    put_cpu();

    if (cpuid == 0)
    {
        /* reserve cpu0 */
        g_thread_cnt--;
        return 0;
    }

    if (cpuid == 1)
    {
        test6_1(param);
        g_thread_cnt--;
        return 0;
    }
    
    per_cpu(g_start_jiffies, cpuid) = jiffies_64;

    while(likely(count < g_max_count))
	{
	    rcu_read_lock();
        count ++;
        rcu_read_unlock();
	}

    per_cpu(g_end_jiffies, cpuid) = jiffies_64;
    printk("[cpu%d]kthread stoped\n", cpuid);

    g_thread_cnt--;
    snprintf(buf, sizeof(buf), "[test5]rcu_read_lock %d thread", num_online_cpus() - 2);
    if (g_thread_cnt == 1)
        print_time(buf);
    return 0;
}

static int test_start(int (*threadfn)(void *data))
{
    int cpuid;
	
	printk(KERN_ALERT"start loop, loop=%llu\n", g_max_count);
    g_thread_cnt = num_online_cpus();

    for_each_online_cpu(cpuid)
    {
        per_cpu(t_thread, cpuid) = kthread_create(threadfn, NULL, "test_thread");
        if (IS_ERR(per_cpu(t_thread, cpuid)))
            return -1;
            
        kthread_bind(per_cpu(t_thread, cpuid), cpuid);
    
        wake_up_process(per_cpu(t_thread, cpuid));   
    }
        
	return 0;
}

static ssize_t proc_test_read(struct file *file, char __user *user, size_t size, loff_t *offset)
{
    int ret = 0;
    char const buff[] = {
            "\t \n\
        0 - \n\
        1 - each cpu access data in one cache line \n\
        2 - each cpu access data in different cache line\n\
        3 - each cpu access data in percpu struct\n\
        4 - test atomic affect with other vary in same cache line\
        5 - test rwlock consume time\
        6 - test rcu lock consume time\n"
            };
    int buf_len = sizeof(buff);
    int read_len = 0;

//    printk("size=%d, offset=%d\n", size, *offset);

    if (*offset >= buf_len)
    {
        read_len = 0;
        goto out;
    }
    read_len = (size < (buf_len - *offset)) ? size : (buf_len - *offset);
    
//    printk("read_len=%d, size=%d, offset=%d\n", read_len, size, *offset);

    ret = copy_to_user(user, &buff[*offset], read_len);
    if (ret != 0)
    {
        read_len = -1;
        goto out;
    }
    
    *offset += read_len;  

out:
    return read_len;
}

static ssize_t
proc_test_write(
    struct file *file,
    const char __user *buffer,
    size_t count,
    loff_t *pos
    )
{
    int ret = 0;
    char tmp[128] = {0};
    int op;

    ret = copy_from_user(tmp, buffer, (sizeof(tmp) < count) ? sizeof(tmp) : count);
    if (ret != 0)
    {
        return -1;
    }

    op = tmp[0] - '0';

    switch (op)
    {
    case 0:
        break;
    case 1:
        test_start(test1);            
        break;

    case 2:
        test_start(test2);            
        break;

    case 3:
        test_start(test3);            
        break;

    case 4:
        test_start(test4);            
        break;

    case 5:
        test_start(test5);            
        break;

    case 6:
        test_start(test6);            
        break;

    default:
        return -1;            
    }

    return 2;
}

static const struct file_operations g_test_ops = {
    .owner = THIS_MODULE,
    .read = proc_test_read,
    .write  = proc_test_write
};

static int hg_init(void)
{
    int ret = 0;
    struct proc_dir_entry *entry;
    
    g_max_count = (u64)max_cnt * BASE_CNT;
	printk(KERN_ALERT"appex test modules loaded,max_count=%llu\n", g_max_count);

    entry = proc_create("test_lock", S_IALLUGO, (&init_net)->proc_net, &g_test_ops);
    if (!entry)
    {
        ret = -ENOMEM;
    }
    
    test_stats.percpu_cnt = alloc_percpu_gfp(int, GFP_ATOMIC);
    if (!test_stats.percpu_cnt)
    {
        ret = -ENOMEM;
    }

    return ret;
}

static void hg_exit(void)
{
//    int cpuid;

 //   for_each_online_cpu(cpuid)
 //   {
 //       kthread_stop(per_cpu(t_thread, cpuid));
 //   }   
	printk(KERN_ALERT"appex test modules exit\n");
    remove_proc_entry ("test_lock", (&init_net)->proc_net);

    while (g_thread_cnt != 0)
    {
        msleep(1000);
        printk(KERN_ALERT"wait for work thread out, g_thread_cnt=%d\n", g_thread_cnt);
    }

    free_percpu(test_stats.percpu_cnt);    
}

MODULE_LICENSE("GPL");

module_init(hg_init);
module_exit(hg_exit);
