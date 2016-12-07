#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <net/tcp.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <net/inet_timewait_sock.h>

extern struct inet_timewait_death_row tcp_death_row;
extern struct proc_dir_entry proc_root;

#define NET_TCP_TIMEWAIT	1009

static int tcp_timewait_2msl = TCP_TIMEWAIT_LEN / HZ;
static int old_twdr_period;
static struct ctl_table_header *tcp_tw_ctl;

int proc_tcp_tw(ctl_table *table, int write, void __user *buffer, size_t *lenp,
        loff_t *ppos)
{
	int ret;
	struct inet_timewait_death_row *twdr = &tcp_death_row;
	
	ret = proc_dointvec(table, write, buffer, lenp, ppos);
	if (write && ret == 0) {
		spin_lock(&twdr->death_lock);
		twdr->period = tcp_timewait_2msl * HZ / INET_TWDR_TWKILL_SLOTS;
		spin_unlock(&twdr->death_lock);	
	}
	
#if(0)
	printk(KERN_ALERT"time_wait_len=%d second, twdr->period=%d ticks, HZ=%d,\n", 
		tcp_timewait_2msl, twdr->period, HZ);
#endif

	return 0;
}


static ctl_table tcp_tw_table[] =
{
	{
		.ctl_name       = NET_TCP_TIMEWAIT,
		.procname       = "tcp_timewait_2msl",
		.data           = &tcp_timewait_2msl,
		.maxlen         = sizeof(tcp_timewait_2msl),
		.mode           = 0644,
		.proc_handler   = &proc_tcp_tw
	},
	{ .ctl_name = 0 }
};

static ctl_table sys_net_ipv4_table[] =
{
	{
		.procname       = "ipv4",
		.mode           = 0555,
		.child          = tcp_tw_table
	},
	{ .ctl_name = 0 }
};


static ctl_table sys_net_table[] =
{
	{
		.ctl_name       = CTL_NET,
		.procname       = "net",
		.mode           = 0555,
		.child          = sys_net_ipv4_table
	},
	{ .ctl_name = 0 }
};


static int tw_init(void)
{
	struct inet_timewait_death_row *twdr = &tcp_death_row;

	spin_lock(&twdr->death_lock);
	old_twdr_period = twdr->period;
	spin_unlock(&twdr->death_lock);

	printk(KERN_ALERT"sys.net.ipv4.tcp_timewait_2msl inited\n");

	tcp_tw_ctl = register_sysctl_table(sys_net_table);
	if (!tcp_tw_ctl) {
		printk("register_sysctl_table error!\n");
		return -1;
	}

	return 0;
}

static void tw_exit(void)
{
	struct inet_timewait_death_row *twdr = &tcp_death_row;

	printk(KERN_ALERT"sys.net.ipv4.tcp_timewait_2msl exit\n");
    
	if (tcp_tw_ctl)
		unregister_sysctl_table(tcp_tw_ctl);

	
	spin_lock(&twdr->death_lock);
	twdr->period = old_twdr_period;
	spin_unlock(&twdr->death_lock);
	
	return;
}


module_init(tw_init);
module_exit(tw_exit);
MODULE_LICENSE("GPL");
