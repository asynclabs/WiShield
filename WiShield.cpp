
/******************************************************************************

  Filename:		WiShield.cpp
  Description:	WiShield library file for the WiShield 1.0

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

extern "C" {
	#include "types.h"
	#include "config.h"
	#include "g2100.h"
	#include "stack.h"
}

#include "WProgram.h"
#include "WiShield.h"

void WiShield::init()
{
	zg_init(GBLBUF, 0);
	attachInterrupt(0, zg_isr, LOW);
}

void WiShield::driver_task()
{
	zg_drv_process();
}

void WiShield::stack_task()
{
	stack_process();
}

// FIXME : return success or failure
void WiShield::server_listen(U16 port)
{
	socket(SOCK_STREAM, port);
}

U8* WiShield::data_available(U16* app_len)
{
	*app_len = stack_app_data();

	return APPBUF;
}

void WiShield::send_data(U16 app_len)
{
	stack_set_app_data(app_len);
}

void WiShield::set_more_data(U8 flag)
{
	stack_set_app_more_data(flag);
}

WiShield WiFi;
