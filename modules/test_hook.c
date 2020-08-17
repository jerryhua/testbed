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

/* Hook position adjust. */
static int rxhook_pos = NF_INET_LOCAL_IN;
module_param(rxhook_pos , int, 0444);

static int txhook_pos = NF_INET_LOCAL_OUT;
module_param(txhook_pos , int, 0444);

/* for hook pri adjust. */
int rxhook_pri  =  NF_IP_PRI_LAST;
module_param(rxhook_pri , int, 0444);

int txhook_pri  =  NF_IP_PRI_LAST - 2;
module_param(txhook_pri , int, 0444);

struct timer_list g_timer; 
rwlock_t g_lock;

static unsigned int
tx_hook(
    unsigned int hook, 
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 23) 
    struct sk_buff *Pskb, 
#else
    struct sk_buff **Pskb, 
#endif
    const struct net_device *in,
    const struct net_device *out,
    int (*okfn)(struct sock *, struct sk_buff *)
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

    if (!eth || !iph)
        goto skb_accept;

	if (iph->protocol == IPPROTO_TCP) {
        tcph = (struct tcphdr*)((char*)iph + (iph->ihl << 2));
        if (ntohs(tcph->source) == 80 || ntohs(tcph->dest) == 80){
    		printk("[%s]:{%pI4:%d -> %pI4:%d} syn=%d,"
                "hookpri=%d,nfctinfo=%d\n", 
                __FUNCTION__, &iph->saddr, ntohs(tcph->source), &iph->daddr, ntohs(tcph->dest), tcph->syn,
                txhook_pri, skb->nfctinfo);
        }
	}	

skb_accept:
    return NF_ACCEPT;
}

static unsigned int
rx_hook(
    unsigned int hook, 
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 23) 
    struct sk_buff *Pskb, 
#else
    struct sk_buff **Pskb, 
#endif
    const struct net_device *in,
    const struct net_device *out,
    int (*okfn)(struct sock *, struct sk_buff *)
    )
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 23) 
    struct sk_buff *skb = Pskb;
#else
    struct sk_buff *skb = *Pskb;
#endif
    struct iphdr *iph;
    struct ethhdr *eth;
    struct tcphdr *tcph;

    if (!skb)
        goto skb_accept;

    iph = ip_hdr(skb);
    eth = eth_hdr(skb);

    if (!eth || !iph)
        goto skb_accept;

	if (iph->protocol == IPPROTO_TCP) {
        tcph = (struct tcphdr*)((char*)iph + (iph->ihl << 2));
        if (ntohs(tcph->source) == 80 || ntohs(tcph->dest) == 80){
    		printk("[%s]:{%pI4:%d -> %pI4:%d} syn=%d,"
                "hookpri=%d,nfctinfo=%d,okfn=%p, nf_bridge=%p, skb->dev=%p, dev=%s"
                "skb->pkt_type=%d\n", 
                __FUNCTION__, &iph->saddr, ntohs(tcph->source), &iph->daddr, ntohs(tcph->dest), tcph->syn,
                rxhook_pri, skb->nfctinfo, okfn, skb->nf_bridge,skb->dev, skb->dev?skb->dev->name:"", 
                skb->pkt_type);
            
        }
	}	

skb_accept:
//    okfn(skb);
//    return NF_STOLEN;
    return NF_ACCEPT;
}

static struct nf_hook_ops testhook[] = {
	{
		.hook		= rx_hook,
		.owner		= THIS_MODULE,
		.pf 		= PF_INET,
		.hooknum	= NF_INET_PRE_ROUTING,
		.priority	= NF_IP_PRI_FIRST
	},
	{
		.hook		= tx_hook,
		.owner		= THIS_MODULE,
		.pf 		= PF_INET,
		.hooknum	= NF_INET_LOCAL_OUT,
		.priority	= NF_IP_PRI_LAST - 1
	}
};

#if 0
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
#endif
static int hg_init(void)
{
    testhook[0].hooknum = rxhook_pos;
    testhook[0].priority = rxhook_pri;
    
    testhook[1].hooknum = txhook_pos;
    testhook[1].priority = txhook_pri;
    
   nf_register_hooks(testhook, ARRAY_SIZE(testhook));
//    rwlock_init(&g_lock);
//	init_timer(&g_timer);

	/* init timer */
//	g_timer.function	= timer_fn;
//	g_timer.data		= 0;
//	mod_timer(&g_timer, jiffies + HZ * 1);
    
	return 0;
}

static void hg_exit(void)
{
	printk(KERN_ALERT"appex hello-world modules exit\n");

//    del_timer(&g_timer);
	nf_unregister_hooks(testhook, ARRAY_SIZE(testhook));

    return;
}


module_init(hg_init);
module_exit(hg_exit);
MODULE_LICENSE("GPL");

