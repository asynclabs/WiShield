
/******************************************************************************

  Filename:		webserver.h
  Description:	Webserver app for the WiShield 1.0

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

#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__

#include "psock.h"

typedef struct webserver_state {
	struct psock p;
	char inputbuf[10];
} uip_tcp_appstate_t;

void webserver_appcall(void);
#ifndef UIP_APPCALL
#define UIP_APPCALL webserver_appcall
#endif /* UIP_APPCALL */

void webserver_init(void);

#endif /* __WEBSERVER_H__ */
