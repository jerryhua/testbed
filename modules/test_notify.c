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

/*dev state event*/
static int _IfDevEvent(struct notifier_block *unused, unsigned long event, void *ptr);
static struct notifier_block gs_IfDevNotifier = {
    .notifier_call = _IfDevEvent
};

static int
_IfDevEvent(
    struct notifier_block *unused,
    unsigned long event,
    void *ptr
    )
{
	printk("dev event=%d\n",  event);
	return 0;
}

static int hg_init(void)
{
	int ret;
	
    ret = register_netdevice_notifier(&gs_IfDevNotifier);
    if (ret)
    {
        ret = -1;
       	printk("register device notify failed(rc: %d)\n", ret);
    }		

	return 0;
}

static void hg_exit(void)
{
	printk(KERN_ALERT"test dev event modules exited\n");
	unregister_netdevice_notifier(&gs_IfDevNotifier);    
	return;
}


module_init(hg_init);
module_exit(hg_exit);
MODULE_LICENSE("GPL");
