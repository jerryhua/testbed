#include <linux/init.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/version.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/if_ether.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/hardirq.h>
#include <net/icmp.h>


/* for hook pri adjust. */
int hook_pri  =  NF_IP_PRI_FIRST ;
module_param(hook_pri , int, 0444);

/* for hook pri adjust. */
int hook_num  =  NF_INET_PRE_ROUTING ;
module_param(hook_num , int, 0444);

struct timer_list g_timer; 
rwlock_t g_lock;

static unsigned int
test_hook_func(
    unsigned int hook, 
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 23) 
    struct sk_buff *Pskb, 
#else
    struct sk_buff **Pskb, 
#endif
    const struct net_device *in,
    const struct net_device *out,
    int (*okfn)(struct sk_buff *)
    )
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 23) 
    struct sk_buff *skb = Pskb;
#else
    struct sk_buff *skb = *Pskb;
#endif
    struct iphdr *iph;
	struct tcphdr *tcph;
    struct ethhdr *eth;

    if (!skb)
        goto skb_accept;

    iph = ip_hdr(skb);
    eth = eth_hdr(skb);
	tcph = tcp_hdr(skb);

    if (!eth || !iph)
        goto skb_accept;

//	if (skb->dev && strncmp((const char *)skb->dev->name, "eth1", strlen("eth1")))
	if (in && strncmp((const char *)in->name, "eth1", strlen("eth1")))
	{
	    write_lock_bh(&g_lock);
		printk("[APPEXHOOK]:receive one packet[hookspri=%d]\n", hook_pri);
		printk("protocol = %u ,okfn=%p\n", skb->protocol, okfn); 
//		dump_stack();
		printk("\n");
        mdelay(1000);
	    write_unlock_bh(&g_lock);
	}
//	if (iph->protocol == IPPROTO_ICMP) {
//		printk("[APXHOOK]: skb=%p,hooknum=%d\n", skb, );
//		icmp_send(skb, ICMP_DEST_UNREACH, ICMP_FRAG_NEEDED, 1000);    
//	}
	

skb_accept:
    return NF_ACCEPT;
}

static unsigned int
test_hook_func1(
    unsigned int hook, 
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 23) 
    struct sk_buff *Pskb, 
#else
    struct sk_buff **Pskb, 
#endif
    const struct net_device *in,
    const struct net_device *out,
    int (*okfn)(struct sk_buff *)
    )
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 23) 
    struct sk_buff *skb = Pskb;
#else
    struct sk_buff *skb = *Pskb;
#endif
    struct iphdr *iph;
    struct ethhdr *eth;

    if (!skb)
        goto skb_accept;

    iph = ip_hdr(skb);
    eth = eth_hdr(skb);

    if (!eth || !iph)
        goto skb_accept;

	if (iph->protocol == IPPROTO_ICMP) {
		printk("[APXHOOK]: skb=%p,hookpri=%d\n", skb, NF_IP_PRI_NAT_DST - 1);
	}	

skb_accept:
    return NF_ACCEPT;
}

static unsigned int
test_hook_func2(
    unsigned int hook, 
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 23) 
    struct sk_buff *Pskb, 
#else
    struct sk_buff **Pskb, 
#endif
    const struct net_device *in,
    const struct net_device *out,
    int (*okfn)(struct sk_buff *)
    )
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 23) 
    struct sk_buff *skb = Pskb;
#else
    struct sk_buff *skb = *Pskb;
#endif
    struct iphdr *iph;
    struct ethhdr *eth;

    if (!skb)
        goto skb_accept;

    iph = ip_hdr(skb);
    eth = eth_hdr(skb);

    if (!eth || !iph)
        goto skb_accept;

	if (iph->protocol == IPPROTO_ICMP) {
		printk("[APXHOOK]: skb=%p,hookpri=%d\n", skb, NF_IP_PRI_NAT_DST + 1);
	}
	
skb_accept:
    return NF_ACCEPT;
}


#if 0
int test()
{
	int a[128];

	prefetch(a);
	return 0;
}
#endif
static struct nf_hook_ops testhook[] = {
	{
	    .hook       = test_hook_func,
	    .owner      = THIS_MODULE,
	    .pf         = PF_INET,
	    //.hooknum    = NF_INET_POST_ROUTING,
	    .hooknum    = NF_INET_PRE_ROUTING,
	    .priority   = NF_IP_PRI_FIRST
	}
#if(0)
	{
		.hook		= test_hook_func1,
		.owner		= THIS_MODULE,
		.pf 		= PF_INET,
		//.hooknum	  = NF_INET_POST_ROUTING,
		.hooknum	= NF_INET_PRE_ROUTING,
		.priority	= NF_IP_PRI_NAT_DST - 1
	},
	{
		.hook		= test_hook_func2,
		.owner		= THIS_MODULE,
		.pf 		= PF_INET,
		//.hooknum	  = NF_INET_POST_ROUTING,
		.hooknum	= NF_INET_PRE_ROUTING,
		.priority	= NF_IP_PRI_NAT_DST + 1
	},
#endif

};

static void timer_fn(unsigned long Data)
{
	struct timeval tv;

	do_gettimeofday(&tv);


	write_lock_bh(&g_lock);	
	
    mod_timer(&g_timer, jiffies + HZ / 2);  
	printk("[timer_fn]\n");
    dump_stack();
   
	write_unlock_bh(&g_lock);
    return;
}

static int hg_init(void)
{
   nf_register_hooks(testhook, ARRAY_SIZE(testhook));
    rwlock_init(&g_lock);
	init_timer(&g_timer);

	/* init timer */
	g_timer.function	= timer_fn;
	g_timer.data		= 0;
	mod_timer(&g_timer, jiffies + HZ * 1);
    
	return 0;
}

static void hg_exit(void)
{
	printk(KERN_ALERT"appex hello-world modules exit\n");

    del_timer(&g_timer);
	nf_unregister_hooks(testhook, ARRAY_SIZE(testhook));

    return;
}


module_init(hg_init);
module_exit(hg_exit);
MODULE_LICENSE("GPL");

