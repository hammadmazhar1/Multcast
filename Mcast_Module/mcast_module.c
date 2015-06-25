#define __KERNEL__
#define MODULE


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/mm.h>
#include <net/ip.h>
#include <net/udp.h>
#include <linux/init.h>
#include <linux/in.h>
#include <linux/inet.h>
#include <net/sock.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <linux/udp.h>
#include <linux/ip.h>

#define IP_HDR_LEN 20
#define UDP_HDR_LEN 8

#define SERVERPORT 1532
#define THRESHOLD 40
#define ACK_THRESH 10

#define MY_ID 3

static struct socket *clientsocket=NULL;
static struct nf_hook_ops nfho;
struct udphdr *udph;
unsigned char* data;
/* Destination for feedback */
struct sockaddr_in to;

/* Multicast address in use */
unsigned int mcast_addr;

/* FeedBack Struct */
int rtable[12] = {0};
int ctable[12] = {0};
/* Current Rate Index */
int cur_idx;

int ack_counter = 0;
int l_received = 0;
int leader = 0;
int counter = 0;

void send_takeover(int t)
{
    int len;
    int i=0;
    char buf[64];
    struct msghdr msg;
    struct iovec iov;
    mm_segment_t oldfs;
    memset(&msg,0,sizeof(msg));
    msg.msg_name = &to;
    msg.msg_namelen = sizeof(to);
    rtable[0] = 99;
    memcpy( buf, rtable, 48 );
    iov.iov_base = buf;
    iov.iov_len  = 48;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    msg.msg_iov    = &iov;
    msg.msg_iovlen = 1;
    // msg.msg_flags    = MSG_NOSIGNAL;

    oldfs = get_fs();
    set_fs( KERNEL_DS );
    len = sock_sendmsg( clientsocket, &msg, 48 );

    for (i=0; i<12; i++)
        rtable[i] = 0;

    set_fs( oldfs );
}
void send_fback(int t)
{
    int len;
    int i=0;
    char buf[64];
    struct msghdr msg;
    struct iovec iov;
    mm_segment_t oldfs;
    memset(&msg,0,sizeof(msg));
    msg.msg_name = &to;
    msg.msg_namelen = sizeof(to);
    memcpy( buf, rtable, 48 );
    iov.iov_base = buf;
    iov.iov_len  = 48;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    msg.msg_iov    = &iov;
    msg.msg_iovlen = 1;
    // msg.msg_flags    = MSG_NOSIGNAL;

    oldfs = get_fs();
    set_fs( KERNEL_DS );
    len = sock_sendmsg( clientsocket, &msg, 48 );

    for (i=0; i<12; i++)
    	rtable[i] = 0;

    set_fs( oldfs );
}
unsigned int hook_func(const struct nf_hook_ops *ops, struct sk_buff *skb, const struct net_device *in, const struct net_device *out, int (*okfn)(struct skbuff*))
{
	
    int i;
    struct iphdr* ip_header;
    unsigned int daddr;
    struct sk_buff *sock_buff = skb;

    ip_header = (struct iphdr*)skb_network_header(sock_buff);
    printk("assigned network header\n");
    daddr = (unsigned int)ip_header->daddr;
    //unsigned int daddr = (unsigned int)((struct iphdr *)skb_network_header(sock_buff))->daddr;    //grab network header using accessor
    
    if(!sock_buff)
    {
    	printk("no packet data\n");
        return NF_ACCEPT;
    }

    if (daddr == mcast_addr)
    {
        unsigned int ttl;
        unsigned int r_ttl;

        udph = (struct udphdr *) skb_header_pointer (skb, IP_HDR_LEN, 0, NULL);
        printk("udp header accessed\n");
        
        // If Ack overheard /
		if (udph->dest == 1532 && leader != MY_ID && leader != 0)
		{
			printk("Overhead packet ack\n");
			/*
			data = (unsigned char *) skb_header_pointer (skb, IP_HDR_LEN+UDP_HDR_LEN, 0, NULL);
			memcpy(ctable, data, 48);

            for (i=0; i < 12; i++)
            {
                l_received += ctable[i];
            }
            ack_counter++;

            // After set Number of Acks, Takeover as leader
             //  If received less than 90% of leader's correct received
            
            if (ack_counter >= ACK_THRESH)
            {
                if (counter < (9*l_received/10))
                    send_takeover(0);
            }
            return NF_ACCEPT;
            */
		}

        ttl = (unsigned int)ip_header->ttl;    //grab network header using accessor
        printk("accessed packet ttl\n");
        
		r_ttl = ttl & 0x1F; 
		printk("ttl:%d\n", ttl);
		printk("r_ttl:%d\n", r_ttl);

		leader = ttl >> 5;
		printk("Leader: %d \n", leader);
		cur_idx = (r_ttl-4)/2;
		printk("cur_idx : %d\n", cur_idx);
        if (cur_idx <0 || cur_idx > 11) {
            printk("TTL incorrectly sent: %d\n", cur_idx);
        } else
		  rtable[cur_idx]++;

		counter++;
		
		if (counter > THRESHOLD && (leader == MY_ID || leader == 0))
		{
			counter = 0;
			printk("Sending feedback\n");

        	send_fback(0);
        }
        else
        	printk("Leader: %d\n", leader);
        

    }
	printk("packet accepted\n");
    return NF_ACCEPT;
}
int init_module(void)
{
	if( sock_create( PF_INET,SOCK_DGRAM,IPPROTO_UDP,&clientsocket)<0 ){
        printk("server: Error creating clientsocket.n" );
        return -EIO;
    }
    memset(&to,0, sizeof(to));
    to.sin_family = AF_INET;
    to.sin_addr.s_addr = in_aton( "224.0.67.67" );

    /* destination address */
    to.sin_port = htons( (unsigned short) SERVERPORT );

    /* Initialize Hook */
    nfho.hook = (nf_hookfn*) hook_func;
    nfho.hooknum = 0;   //NF_IP_PRE_ROUTING;
    nfho.pf = PF_INET;
    nfho.priority = NF_IP_PRI_FIRST;

    mcast_addr = in_aton("224.0.67.67");

    nf_register_hook(&nfho);

    send_fback(0);
    printk("Hook Registered\n");
    return 0;
}

void cleanup_module(void)
{
	 if( clientsocket )
        sock_release( clientsocket );
    nf_unregister_hook(&nfho);
    printk("Hook Unregistered\n");
}