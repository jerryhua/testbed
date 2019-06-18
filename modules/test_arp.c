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
    struct net_device *dev = dev_get_by_name(&init_net, "eth0");
    struct neighbour *neigh = NULL;
	
	printk(KERN_ALERT"test arp modules inited\n");
		
	neigh = __neigh_lookup(&arp_tbl, "10.1.1.105", dev, 1);
	printk(KERN_ALERT"neigh=%p\n", neigh);

	return -1;
}

static void hg_exit(void)
{
	printk(KERN_ALERT"test arp modules exited\n");
    
	return;
}


module_init(hg_init);
module_exit(hg_exit);
MODULE_LICENSE("GPL");
