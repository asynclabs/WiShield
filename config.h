
/******************************************************************************

  Filename:		config.h
  Description:	Wireless configuration parameters for the WiShield 1.0

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
   AsyncLabs			05/01/2009	Initial version

 *****************************************************************************/

#ifndef CONFIG_H_
#define CONFIG_H_

#include "types.h"

#define IP_ADDR(a,b,c,d)	((((U32)d)<<24)|(((U32)c)<<16)|(((U32)b)<<8)|(a))

extern U32 local_ip;
extern const prog_char ssid[];
extern U8 ssid_len;
extern const prog_char security_passphrase[];
extern U8 security_passphrase_len;
extern U8 security_type;
extern U8 wireless_mode;

extern prog_uchar wep_keys[];

extern const prog_char webpage[];

extern U8 buf[];

#define GBLBUF		((U8*)buf)

#define WIRELESS_MODE_INFRA	1
#define WIRELESS_MODE_ADHOC	2

#endif /* CONFIG_H_ */
