
/******************************************************************************

  Filename:		clock-arch.c
  Description:	Timer routine file

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
   AsyncLabs			05/29/2009	Initial port

 *****************************************************************************/

#include "global-conf.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sfr_defs.h>

#include "clock-arch.h"
#include "wiring.h"

#if 0
//Counted time
clock_time_t clock_datetime = 0;

//Overflow interrupt
ISR(TIMER0_OVF_vect)
{
	clock_datetime += 1;
	TIFR0 |= (1<<TOV0);
}
*/

//Initialise the clock
void clock_init(){
	//Activate overflow interrupt for timer0
	TIMSK0 |= (1<<TOIE0);

	//Use prescaler 1024
	TCCR0B |= ((1<<CS12)|(1<<CS10));

	//Activate interrupts
	sei();
}

//Return time
clock_time_t clock_time(){
	clock_time_t time;

	cli();
	time = clock_datetime;
	sei();

	return time;
}
#endif

//Return time
clock_time_t clock_time()
{
	return millis();
}
