// Copy Ethernet addresses Tomorrow */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <linux/udp.h>
#include <linux/ip.h>
#include <linux/inet.h>
 
static struct nf_hook_ops nfho;   //net filter hook option struct
struct sk_buff *sock_buff;
struct udphdr *udp_header;          //udp header struct 
struct iphdr *ip_header;            //ip header struct
struct inet_sock *inet;
struct ethhdr *eth_header;
struct ethhdr *eth;
unsigned char* data;

struct sk_buff *skb;
struct in_addr temp;

int counter;

unsigned char ea1[7] = {0x00,0x07,0xe9,0x24,0x71,0x15};

unsigned int daddr;
unsigned int mcast_addr;

void print_mac_hdr(struct ethhdr *eth);

unsigned int hook_func(unsigned int hooknum, struct sk_buff *skb, const struct net_device *in, const struct net_device *out, int (*okfn)(struct sk_buff *));

int len;

void sendAck(unsigned int dest_address, struct ethhdr* ethheader)
{
    len = 128;

    skb = alloc_skb(len, GFP_ATOMIC);

    if (!skb)
    {
        printk("Skb allocation failed\n");   
        return; 
    }
    /* 
        Note: If I don't reserve enough space the module crashes. 
        UDP header size is 8 Bytes + 20 Bytes IP Header Size + 14 Bytes Eth Header
    */

    skb_reserve(skb, 42);

    data = skb_put(skb, len);

    skb_push(skb, sizeof(struct udphdr));
    
    skb_reset_transport_header(skb);

    /*
        Craft UDP Header
    */

    udp_header = udp_hdr(skb);
    udp_header->source = mcast_addr;
    udp_header->dest = daddr;
    udp_header->len = len; // Check this incase packet isn't sent properly
    udp_header->check = 0;
    udp_header->check = csum_tcpudp_magic(mcast_addr, daddr, len, IPPROTO_UDP, csum_partial(udp_header, len, 0));
    
    if (udp_header->check == 0)
    {
        udp_header->check = CSUM_MANGLED_0;
    }

    /* 
        Craft IP Header
    */
    inet = inet_sk(skb->sk);
    skb_push(skb, sizeof(struct iphdr));
    skb_reset_network_header(skb);
    ip_header = ip_hdr(skb);
    
    ip_header->version = 4;
    ip_header->ihl = 5;
    ip_header->tos = 0;
    ip_header->tot_len = skb->len;
    ip_header->frag_off = 0;

    
    ip_header->id = 1532;
    
    ip_header->ttl = 64;
    ip_header->protocol = IPPROTO_UDP;
    ip_header->check = 0;

    ip_header->saddr = mcast_addr;
    ip_header->daddr = dest_address;
    ip_header->check = ip_fast_csum((unsigned char*)ip_header, ip_header->ihl);
   

    
    eth = (struct ethhdr*)skb_push(skb, ETH_HLEN);
    skb_reset_mac_header(skb);
    skb->protocol = eth->h_proto = htons(ETH_P_IP);
    memcpy(eth->h_source, ethheader->h_dest, ETH_ALEN);
    memcpy(eth->h_dest, ethheader->h_source, ETH_ALEN);

    if ((skb->dev = (struct net_device*)(dev_get_by_name(&init_net, "wlan0"))) == NULL)
    {
        printk("ERROR HERE\n");
        return;
    }

    skb->pkt_type = PACKET_OUTGOING;

    dev_queue_xmit(skb);
} 

int init_module()
{
        nfho.hook = (nf_hookfn*) hook_func;
        nfho.hooknum = 0;   //NF_IP_PRE_ROUTING;
        nfho.pf = PF_INET;
        nfho.priority = NF_IP_PRI_FIRST;
        counter = 0;

        mcast_addr = in_aton("224.0.67.67");

        nf_register_hook(&nfho);
        printk("Hook Registered\n");
        return 0;
}

void cleanup_module()
{
        printk("Hook Unregistered\n");
        nf_unregister_hook(&nfho);     
}

unsigned int hook_func(unsigned int hooknum, struct sk_buff *skb, const struct net_device *in, const struct net_device *out, int (*okfn)(struct sk_buff *))
{
        sock_buff = skb;
 
        daddr = (unsigned int)((struct iphdr *)skb_network_header(sock_buff))->daddr;    //grab network header using accessor
        eth_header = eth_hdr(skb);

        sendAck((unsigned int)((struct iphdr *)skb_network_header(sock_buff))->saddr, eth_header);

        if(!sock_buff) 
        { 
            return NF_ACCEPT;
        }
        
        if (daddr == mcast_addr)
        {
            counter++;
            
            if (counter%27 == 0)
            {
                print_mac_hdr(eth_header);
                sendAck((unsigned int)((struct iphdr *)skb_network_header(sock_buff))->saddr, eth_header);
            }
        }
        
        return NF_ACCEPT;
}

void print_mac_hdr(struct ethhdr *eth)
{
    printk("Destino: %02x:%02x:%02x:%02x:%02x:%02x \n",eth->h_dest[0],eth->h_dest[1],eth->h_dest[2],eth->h_dest[3],eth->h_dest[4],eth->h_dest[5]);
    printk("Origem: %02x:%02x:%02x:%02x:%02x:%02x\n",eth->h_source[0],eth->h_source[1],eth->h_source[2],eth->h_source[3],eth->h_source[4],eth->h_source[5]);
    printk("Proto: 0x%04x\n",ntohs(eth->h_proto));

}