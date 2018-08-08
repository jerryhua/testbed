#include <linux/init.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/version.h>
#include <linux/ip.h>
#include <linux/if_ether.h>
#include <net/net_namespace.h>
#include <linux/hash.h>
#include <linux/ip.h>
#include <linux/if_arp.h>
#include <net/protocol.h>
#include <net/xfrm.h>
#include <net/udp.h>
#include <net/flow.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <net/inet_timewait_sock.h>
#include <net/neighbour.h>
#include <net/arp.h>
int type;
module_param(type, int, 0644);
MODULE_PARM_DESC(type, "lock type  (0-lock/1-lock_bh/2-lock_irq)");

/*
static char *type;
module_param(type, charp, 0644);
MODULE_PARM_DESC(type, "lock type  (lock/lock_bh/lock_irq)");
*/
static int hg_init(void)
{
	unsigned long jf, jf2;
	rwlock_t lock;
	unsigned long irq_flags = 0;
	
	
	printk(KERN_ALERT"test lock modules inited\n");
	rwlock_init(&lock);
		
	if (type == 0)
		read_lock(&lock);
	else if (type == 1)
		read_lock_bh(&lock);
	else if (type == 2)
		read_lock_irqsave(&lock, irq_flags);

	jf = jiffies;
	jf2 = jiffies + 40 * HZ;
	printk(KERN_ALERT"jf=%lu, jf2=%lu\n", jf, jf2);
	while(!time_after(jiffies, jf2));
	printk(KERN_ALERT"jiffies=%lu\n", jiffies);

	if (type == 0)
		read_unlock(&lock);
	else if (type == 1)
		read_unlock_bh(&lock);
	else if (type == 2)
		read_unlock_irqrestore(&lock, irq_flags);

	return -1;
}

static void hg_exit(void)
{
	printk(KERN_ALERT"test lock modules exited\n");
    
	return;
}


module_init(hg_init);
module_exit(hg_exit);
MODULE_LICENSE("GPL");
