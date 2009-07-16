
/******************************************************************************

  Filename:		network.c
  Description:	Network interface for the WiShield 1.0

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

#include "global-conf.h"
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include "witypes.h"
#include "g2100.h"

void network_init(void)
{
}

unsigned int network_read(void)
{
	return zg_get_rx_status();
}

void network_send(void)
{
	if (uip_len > 0) {
		if(uip_len <= UIP_LLH_LEN + 40){
			zg_set_buf(uip_buf, uip_len);
		}
		else{
			memcpy((u8*)&uip_buf[54], (u8*)uip_appdata, (uip_len-54));
			zg_set_buf(uip_buf, uip_len);
		}
		zg_set_tx_status(1);
	}
}

void network_get_MAC(u8* macaddr)
{
}

void network_set_MAC(u8* macaddr)
{
}
