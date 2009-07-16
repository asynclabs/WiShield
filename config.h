
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
   AsyncLabs			05/29/2009	Adding support for new library

 *****************************************************************************/

#ifndef CONFIG_H_
#define CONFIG_H_

#include "witypes.h"

extern U8 local_ip[];
extern U8 gateway_ip[];
extern U8 subnet_mask[];
extern const prog_char ssid[];
extern U8 ssid_len;
extern const prog_char security_passphrase[];
extern U8 security_passphrase_len;
extern U8 security_type;
extern U8 wireless_mode;

extern prog_uchar wep_keys[];

extern const prog_char webpage[];
extern const prog_char twitter[];

#define WIRELESS_MODE_INFRA	1
#define WIRELESS_MODE_ADHOC	2

#endif /* CONFIG_H_ */
