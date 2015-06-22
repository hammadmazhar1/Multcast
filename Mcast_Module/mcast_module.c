#define __KERNEL__
#define MODULE


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

static struct nf_hook_ops nfho;

unsigned int hook_func(unsigned int hooknum, struct sk_buff **skb, const struct net_device *in, const struct net_device *out, int (*okfn)(struct skbuff*))
{
	printk(KERN_INFO "packet dropped\n");
	return NF_DROP;
}

int init_module(void)
{
	nfho.hook = hook_func;
	nfho.hooknum = 0;  //NF_IP_PRE_ROUTING
	nfho.pf = PF_INET;
	nfho.priority = NF_IP_PRI_FIRST;
	nf_register_hook(&nfho);
	printk(KERN_INFO "init_module() called\n");
	return 0;
}

void cleanup_module(void)
{
	nf_unregister_hook(&nfho);
	printk(KERN_INFO "cleanup_module() called\n");
}