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

static int tcp_tw_timeout;
static int twdr_period_old;
void (* inet_twdr_hangman_old)(unsigned long data);
static struct ctl_table_header *tcp_tw_ctl;


void inet_twdr_hangman_timer_fn(unsigned long data);

struct inet_timewait_death_row tcp_death_row_tw = {
	.sysctl_max_tw_buckets = NR_FILE * 2,
	.period		= TCP_TIMEWAIT_LEN / INET_TWDR_TWKILL_SLOTS,
	.death_lock	= __SPIN_LOCK_UNLOCKED(tcp_death_row_tw.death_lock),
	.hashinfo	= &tcp_hashinfo,
	.tw_timer	= TIMER_INITIALIZER(inet_twdr_hangman_timer_fn, 0,
					    (unsigned long)&tcp_death_row_tw),
	.twkill_work	= __WORK_INITIALIZER(tcp_death_row.twkill_work,
					     inet_twdr_twkill_work),
};

void inet_twdr_hangman_timer_fn(unsigned long data)
{
	int fin_slot, tw_slot;
	int need_tw_timer;
	struct hlist_head *list;
	struct inet_timewait_sock *tw;
	struct hlist_node *node;

	fin_slot = INET_TWDR_TWKILL_SLOTS - 1;
	spin_lock(&tcp_death_row_tw.death_lock);
	spin_lock(&tcp_death_row.death_lock);

	/* locate the source slot and dest slot
	*   I am not sure whether we need traverse tcp_death_row.slot.
	*/
	fin_slot = (tcp_death_row.slot + fin_slot) & (INET_TWDR_TWKILL_SLOTS - 1);
	tw_slot = (tcp_death_row_tw.slot + fin_slot) & (INET_TWDR_TWKILL_SLOTS - 1);

	/* move time_wait sock from tcp_death_row to tcp_death_row_tw */
	inet_twsk_for_each_inmate(tw, node, &tcp_death_row.cells[fin_slot]) {
		if (tw->tw_substate == TCP_TIME_WAIT) {
			__inet_twsk_del_dead_node(tw);			
			list = &tcp_death_row_tw.cells[tw_slot];
			hlist_add_head(&tw->tw_death_node, list);
			
			if (tcp_death_row_tw.tw_count++ == 0)
				need_tw_timer = 1;
		}
	}
	
	spin_unlock(&tcp_death_row.death_lock);	
	spin_unlock(&tcp_death_row_tw.death_lock);

	if (need_tw_timer)
		mod_timer(&tcp_death_row_tw.tw_timer, 
			jiffies + tcp_death_row_tw.period);
	
	inet_twdr_hangman(data);
}

int proc_tcp_tw(ctl_table *table, int write, void __user *buffer, size_t *lenp,
        loff_t *ppos)
{
	int ret;
	struct inet_timewait_death_row *twdr = &tcp_death_row_tw;
	
	ret = proc_dointvec(table, write, buffer, lenp, ppos);
	if (write && ret == 0) {
		spin_lock(&twdr->death_lock);
		if (tcp_tw_timeout * HZ > TCP_TIMEWAIT_LEN)
			twdr->period = TCP_TIMEWAIT_LEN / INET_TWDR_TWKILL_SLOTS;
		else
			twdr->period = tcp_tw_timeout * HZ / INET_TWDR_TWKILL_SLOTS;
		spin_unlock(&twdr->death_lock);	
	}
	
#if(0)
	printk(KERN_ALERT"time_wait_len=%d second, twdr->period=%d ticks, HZ=%d,\n", 
		tcp_tw_timeout, twdr->period, HZ);
#endif

	return 0;
}


static ctl_table tcp_tw_table[] =
{
	{
		.ctl_name       = NET_TCP_TIMEWAIT,
		.procname       = "tcp_tw_timeout",
		.data           = &tcp_tw_timeout,
		.maxlen         = sizeof(tcp_tw_timeout),
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
	twdr_period_old = twdr->period;
	tcp_tw_timeout = twdr->period * INET_TWDR_TWKILL_SLOTS / HZ;
	inet_twdr_hangman_old = twdr->tw_timer.function;
	twdr->tw_timer.function = inet_twdr_hangman_timer_fn;
	spin_unlock(&twdr->death_lock);

	printk(KERN_ALERT"sys.net.ipv4.tcp_tw_timeout inited\n");

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

	printk(KERN_ALERT"sys.net.ipv4.tcp_tw_timeout exit\n");
    
	if (tcp_tw_ctl)
		unregister_sysctl_table(tcp_tw_ctl);

	
	spin_lock(&twdr->death_lock);
	twdr->period = twdr_period_old;
	twdr->tw_timer.function = inet_twdr_hangman_old;
	spin_unlock(&twdr->death_lock);
	
	return;
}


module_init(tw_init);
module_exit(tw_exit);
MODULE_LICENSE("GPL");
