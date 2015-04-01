#include <linux/init.h>   
#include <linux/kernel.h>   
#include <linux/module.h>   
#include <linux/netdevice.h>  
#include <linux/skbuff.h>  
#include <linux/ip.h>
#include <linux/udp.h>

#include <net/net_namespace.h> /* for use of init_net*/

#include <linux/timer.h>
#include <linux/time.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Wang Qi, jameryw@gmail.com");
MODULE_VERSION("v1.0");
MODULE_DESCRIPTION("A very high-speed packets sender, which can be used to test the performance of NIC or do something evil..");
MODULE_ALIAS("VHSPS");

int HighSpeedSender_sendpackets(const char *devname,long long num)
{
	struct sk_buff *skb=NULL;
	struct sk_buff *oldskb=NULL;
	struct net_device *dev=NULL;
	struct timeval tv; 
	long long L1,L2;
	long long count=0;
	u_char pkt_eth [14] = 
	{
	0xe0, 0xcb, 0x4e, 0x2a, 0x34, 0xae, //dstmac
	0x00, 0x19, 0x21, 0x32, 0x13, 0x14, //srcmac
	0x08, 0x00 			    //protocal type
	}; //eth
	u_char pkt_ip [20] = 
	{
	0x45, 0x00,		//version, tos
	0x00, 0x32, 0x01, 0x02, //ip length, id
	0x00, 0x00, 		//flag, fragment offset
	0x40, 0x11, 		//ttl, protocal type
	0x00, 0x00, 		//checksum
	0xC0, 0xA8, 0x02, 0x0d, //srcip
	0xC0, 0xA8, 0x02, 0x0b	//dstip
	}; //ip
	u_char pkt_udp [8] = 
	{
	0x01, 0x00, //srcport
	0x00, 0x10, //dstport
	0x00, 0x1e, //udp length
	0x00, 0x00  //checksum
	}; //udp
	u_char pkt_data [22] = 
	{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
	0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 
	0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 
	};//data

	dev=dev_get_by_name(&init_net,devname);
	if(dev==NULL) 
	{
		printk(KERN_WARNING "HighSpeedSender No such device:%s!!\n",devname);
		return -1;
	}
	printk(KERN_WARNING "HighSpeedSender Device name:%s\n",dev->name);

	skb = alloc_skb (64+LL_RESERVED_SPACE(dev), GFP_ATOMIC);
	if(skb==NULL) 
	{
		printk(KERN_WARNING "HighSpeedSender Allocate sk_buff error!!\n");
		return -1;
	}
	skb->dev=dev;
	dev_put(skb->dev);
	skb->pkt_type = PACKET_OTHERHOST;//PACKET_OUTGOING;
	skb->protocol = __constant_htons(ETH_P_IP);
	skb->ip_summed = CHECKSUM_NONE;
	skb->priority = 0;

	skb_reserve(skb, LL_RESERVED_SPACE(dev));

	memcpy (skb_push (skb, 14), pkt_eth, 14);

	skb->network_header = skb_put (skb, sizeof (struct iphdr));
	memcpy (skb->network_header, pkt_ip, sizeof (struct iphdr));

	skb->transport_header = skb_put (skb, sizeof (struct udphdr));
	memcpy (skb->transport_header, pkt_udp, sizeof (struct udphdr));

	memcpy (skb_put (skb, 22), pkt_data, 22);

	skb->tail = skb->data + 64;

	oldskb=skb_copy(skb,GFP_ATOMIC);

	printk(KERN_WARNING "HighSpeedSender Sending packets......\n");
	do_gettimeofday(&tv);
	L1 = tv.tv_sec*1000*1000 + tv.tv_usec;
	if(num>0)
	{
		while(count!=num)
		{
			if(dev->netdev_ops->ndo_start_xmit(skb, dev)!=NETDEV_TX_OK)
			{
				//printk(KERN_WARNING "HighSpeedSender Sending packets error!! %lld packets sended\n",count);
				continue;
			}
			skb=skb_copy(oldskb,GFP_ATOMIC);
			count++;
		}
	}else
	{
		while(1)
		{
			if(dev->netdev_ops->ndo_start_xmit(skb, dev)!=NETDEV_TX_OK)
			{
				//printk(KERN_WARNING "HighSpeedSender Sending packets error!! %lld packets sended\n",count);
				continue;
			}
			skb=skb_copy(oldskb,GFP_ATOMIC);
			count++;
		}
	}
	do_gettimeofday(&tv);
	L2 = tv.tv_sec*1000*1000 + tv.tv_usec;
	kfree_skb(skb);
	
	printk(KERN_WARNING "HighSpeedSender Sending packets finished\n");
	printk(KERN_WARNING "HighSpeedSender Sended %lld packets successfully! Duration:%lld ms\n",count,L2-L1);

	return 0;
}

static int HighSpeedSender_init(void) 
{  
    printk(KERN_WARNING "HighSpeedSender module init!\n");  
    HighSpeedSender_sendpackets("eth0",10000000);
    return 0;  
}
  
static void HighSpeedSender_exit(void) 
{  
    printk(KERN_WARNING "HighSpeedSender module exit!\n");  
}  
  
module_init(HighSpeedSender_init);  
module_exit(HighSpeedSender_exit);  
