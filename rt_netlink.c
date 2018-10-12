/*--------------------------------------------------------
 *  Copyright(C) 2018 EASTCOM-BUPT Inc.
 *
 *  Author      : even li
 *  Description : even li at ebupt.com
 *  History     : 2015-06-22 Created
 *
 *   Module Name:
 *   rt_netlink.c
 *
 *   Abstract:
 *   Create and register netlink system for ralink device
 *
 *--------------------------------------------------------
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <net/sock.h>
#include <linux/netlink.h>

#include "rt_netlink.h"

#define NETLINK_TEST 17 

struct {
    __u32 pid;
}user_process;

static struct sock *netlinkfd = NULL;
int send_message_to_user(char *message);
int send_detectdata_to_user(unsigned char *mac, wifi_detect detect);
/********************************************************************
description:
	send data to user netlink, e.x mac rssi snr

argu:
	mac: mac of sta
	detect: subtype,frametype,rssi snr.....

return:
	0:                       sucess
	other:                 fail

autor:
	evenli@gmail.com

date:
	10/08/2015

********************************************************************/
int send_message_to_user(char *message){
	int size;
    struct sk_buff *skb;
    unsigned char *old_tail;
    struct nlmsghdr *nlh; 

    int retval;

    size = NLMSG_SPACE(strlen(message)); 
    skb = alloc_skb(size, GFP_ATOMIC); 
    nlh = nlmsg_put(skb, 0, 0, 0, NLMSG_SPACE(strlen(message))-sizeof(struct nlmsghdr), 0); 
    old_tail = skb->tail;
    
    memcpy(NLMSG_DATA(nlh), message, strlen(message));
    nlh->nlmsg_len = skb->tail - old_tail; 


    NETLINK_CB(skb).dst_group = 1;

    retval = netlink_broadcast(netlinkfd, skb, 0, 1, GFP_ATOMIC);

    return 0;
}
/********************************************************************
description:
	send the detect data to user netlink, e.x mac rssi snr

argu:
	mac: mac of sta
	detect: subtype,frametype,rssi snr.....

return:
	0:                       sucess
	other:                 fail

autor:
	evenli@gmail.com

date:
	10/08/2015

********************************************************************/
int send_detectdata_to_user(unsigned char *mac, wifi_detect detect){
	int size;
    struct sk_buff *skb;
    unsigned char *old_tail;
    struct nlmsghdr *nlh;

    int retval;

    unsigned char info[16];
    memset(info, 0, 16);
    info[0] = detect.frametype;
    info[1] = detect.subtype;
    
    memcpy(&info[2], mac, 6);
    info[8] = detect.rssi0;
    info[9] = detect.rssi1;
    info[10] = detect.rssi2;
	
    info[11] = detect.snr0;
    info[12] = detect.snr1;
    info[13] = detect.snr2;

    size = NLMSG_SPACE(strlen(info)); 
    skb = alloc_skb(size, GFP_ATOMIC); 
    nlh = nlmsg_put(skb, 0, 0, 0, NLMSG_SPACE(strlen(info))-sizeof(struct nlmsghdr), 0); 
    old_tail = skb->tail;
    
    memcpy(NLMSG_DATA(nlh), info, strlen(info));
    nlh->nlmsg_len = skb->tail - old_tail; 

    NETLINK_CB(skb).dst_group = 1;

    retval = netlink_broadcast(netlinkfd, skb, 0, 1, GFP_ATOMIC);

    return 0;
}

void kernel_receive(struct sk_buff *__skb) {
    struct sk_buff *skb;
    struct nlmsghdr *nlh = NULL;

    char *data = "message from kernel";

    skb = skb_get(__skb);

    if(skb->len >= sizeof(struct nlmsghdr)){
        nlh = (struct nlmsghdr *)skb->data;
        if((nlh->nlmsg_len >= sizeof(struct nlmsghdr))
            && (__skb->len >= nlh->nlmsg_len)){
            user_process.pid = nlh->nlmsg_pid;
            if (strcmp((char *)NLMSG_DATA(nlh), "g") == 0){
				send_message_to_user(data);
            }
        }
    }else{
        printk(KERN_ERR "[kernel space]2 data receive from user are:%s\n",(char *)NLMSG_DATA(nlmsg_hdr(__skb)));
    }

    kfree_skb(skb);
}

int rt_netlink_init(void){
	struct netlink_kernel_cfg cfg = {
    .input = kernel_receive,
	};
		
    netlinkfd = netlink_kernel_create(&init_net, NETLINK_TEST,  &cfg);
    if(!netlinkfd){
        printk(KERN_ERR "can not create a netlink socket\n");
        return -1;
    }
	printk("[kernel space] rt_netlink_init\n");
    return 0;
}

int rt_netlink_exit(void){
	sock_release(netlinkfd->sk_socket);
	printk("[kernel space]rt_netlink_exit\n");
}


