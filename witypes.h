
/******************************************************************************

  Filename:		witypes.h
  Description:	Data types

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

#ifndef WITYPES_H_
#define WITYPES_H_

#include <avr/pgmspace.h>

#ifndef NULL
#define NULL		((void *) 0)
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif

typedef char int8;
typedef volatile char vint8;
typedef unsigned char uint8;
typedef volatile unsigned char vuint8;
typedef int int16;
typedef volatile int vint16;
typedef unsigned int uint16;
typedef volatile unsigned int vuint16;
typedef long int32;
typedef volatile long vint32;
typedef unsigned long uint32;
typedef volatile unsigned long vuint32;

typedef unsigned char u8;
typedef unsigned char U8;
typedef unsigned int u16;
typedef unsigned int U16;
typedef unsigned long u32;
typedef unsigned long U32;

typedef uint8_t boolean;

#endif /* WITYPES_H_ */
