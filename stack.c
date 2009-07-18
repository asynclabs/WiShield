
/******************************************************************************

  Filename:		stack.c
  Description:	Stack process for the WiShield 1.0

 ******************************************************************************

  TCP/IP stack and driver for the WiShield 1.0 wireless devices

  Copyright(c) 2009 Async Labs Inc. All rights reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston, MA  02111-1307, USA.

  Contact Information:
  <asynclabs@asynclabs.com>

   Author               Date        Comment
  ---------------------------------------------------------------
   AsyncLabs			05/29/2009	Initial version

 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>

#include "timer.h"
#include "global-conf.h"
#include "uip_arp.h"
#include "network.h"
#include "witypes.h"
#include "config.h"
#include "g2100.h"
#include "spi.h"

#include <string.h>
#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

static struct timer periodic_timer, arp_timer, self_arp_timer;

void stack_init(void)
{
	uip_ipaddr_t ipaddr;

	struct uip_eth_addr mac;

	U8* mac_addr = zg_get_mac();

	mac.addr[0] = mac_addr[0];
	mac.addr[1] = mac_addr[1];
	mac.addr[2] = mac_addr[2];
	mac.addr[3] = mac_addr[3];
	mac.addr[4] = mac_addr[4];
	mac.addr[5] = mac_addr[5];

	timer_set(&periodic_timer, CLOCK_SECOND / 2);
	timer_set(&arp_timer, CLOCK_SECOND * 10);
	timer_set(&self_arp_timer, CLOCK_SECOND * 30);

	uip_init();

	uip_setethaddr(mac);

#ifdef APP_WEBSERVER
	webserver_init();
#endif

#ifdef APP_WEBCLIENT
	webclient_init();
#endif

#ifdef APP_SOCKAPP
	socket_app_init();
#endif

#ifdef APP_UDPAPP
	udpapp_init();
#endif

	uip_ipaddr(ipaddr, local_ip[0], local_ip[1], local_ip[2], local_ip[3]);
	uip_sethostaddr(ipaddr);
	uip_ipaddr(ipaddr, gateway_ip[0],gateway_ip[1],gateway_ip[2],gateway_ip[3]);
	uip_setdraddr(ipaddr);
	uip_ipaddr(ipaddr, subnet_mask[0],subnet_mask[1],subnet_mask[2],subnet_mask[3]);
	uip_setnetmask(ipaddr);
}

void stack_process(void)
{
	int i;

		uip_len = network_read();

		if(uip_len > 0) {
			if(BUF->type == htons(UIP_ETHTYPE_IP)){
				uip_arp_ipin();
				uip_input();
				if(uip_len > 0) {
					uip_arp_out();
					network_send();
				}
			}else if(BUF->type == htons(UIP_ETHTYPE_ARP)){
				uip_arp_arpin();
				if(uip_len > 0){
					network_send();
				}
			}

		}else if(timer_expired(&periodic_timer)) {
			timer_reset(&periodic_timer);

			for(i = 0; i < UIP_CONNS; i++) {
				uip_periodic(i);
				if(uip_len > 0) {
					uip_arp_out();
					network_send();
				}
			}

			// if nothing to TX and the self ARP timer expired
			// TX a broadcast ARP reply. This was implemented to
			// cause periodic TX to prevent the AP from disconnecting
			// us from the network
			if (uip_len == 0 && timer_expired(&self_arp_timer)) {
				timer_reset(&self_arp_timer);
				uip_self_arp_out();
				network_send();
			}

			if(timer_expired(&arp_timer)) {
				timer_reset(&arp_timer);
				uip_arp_timer();
			}
		}
	return;
}

void uip_log(char *m)
{
	//TODO: Get debug information out here somehow, does anybody know a smart way to do that?
}
