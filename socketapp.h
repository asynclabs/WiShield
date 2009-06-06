
/******************************************************************************

  Filename:		socketapp.h
  Description:	Simple socket programming example for the WiShield 1.0

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
   AsyncLabs			06/06/2009	Initial version

 *****************************************************************************/

#ifndef __SOCKET_APP_H__
#define __SOCKET_APP_H__

/* Since this file will be included by uip.h, we cannot include uip.h
   here. But we might need to include uipopt.h if we need the u8_t and
   u16_t datatypes. */
#include "uipopt.h"
#include "psock.h"

/* Next, we define the uip_tcp_appstate_t datatype. This is the state
   of our application, and the memory required for this state is
   allocated together with each TCP connection. One application state
   for each TCP connection. */
typedef struct socket_app_state {
  struct psock p;
  char inputbuffer[20];
  char name[20];
} uip_tcp_appstate_t;

/* Finally we define the application function to be called by uIP. */
void socket_app_appcall(void);
#ifndef UIP_APPCALL
#define UIP_APPCALL socket_app_appcall
#endif /* UIP_APPCALL */

void socket_app_init(void);

#endif /* __SOCKET_APP_H__ */
