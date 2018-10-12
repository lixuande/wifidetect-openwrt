/*--------------------------------------------------------
 *  Copyright(C) 2015 EASTCOM-BUPT Inc.
 *
 *  Author      : even li
 *  Description : even li at ebupt.com
 *  History     : 2015-06-22 Created
 *
 *--------------------------------------------------------
*/

#ifndef RT_NETLINK_H
#define RT_NETLINK_H

#include <net/netlink.h> 

typedef struct {
	unsigned char subtype;
	unsigned char frametype;
	char rssi0;
	char rssi1;
	char rssi2;
	char snr0;
	char snr1;
	char snr2;
}wifi_detect;

int send_message_to_user(char *message);
int send_detectdata_to_user(unsigned char *mac, wifi_detect detect);

int rt_netlink_init(void);
int rt_netlink_exit(void);


#endif