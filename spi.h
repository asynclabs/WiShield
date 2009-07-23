
/******************************************************************************

  Filename:		spi.h
  Description:	SPI bus configuration for the WiShield 1.0

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

#ifndef SPI_H_
#define SPI_H_

// Uncomment one line below to
// specify which Arduino pin
// to use as WiShield interrupt
#define USE_DIG0_INTR		// use digital pin 0
//#define USE_DIG8_INTR		// use digital pin 8


#ifdef USE_DIG0_INTR
#define ZG2100_ISR_DISABLE()	(EIMSK &= ~(0x01))
#define ZG2100_ISR_ENABLE()		(EIMSK |= 0x01)
#define ZG2100_ISR_GET(X)		(X = EIMSK)
#define ZG2100_ISR_SET(X)		(EIMSK = X)
#endif

#ifdef USE_DIG8_INTR
#define ZG2100_ISR_DISABLE()	(PCMSK0 &= ~(0x01))
#define ZG2100_ISR_ENABLE()		(PCMSK0 |= 0x01)
#define ZG2100_ISR_GET(X)		(X = PCMSK0)
#define ZG2100_ISR_SET(X)		(PCMSK0 = X)
#endif

//AVR Mega168 SPI HAL
#define BIT0							0x01
#define BIT1							0x02
#define BIT2							0x04
#define BIT3							0x08
#define BIT4							0x10
#define BIT5							0x20
#define BIT6							0x40
#define BIT7							0x80

#ifdef USE_DIG8_INTR
#define ZG2100_INTR						BIT0
#endif

#define SPI0_SS_BIT						BIT2
#define SPI0_SS_DDR						DDRB
#define SPI0_SS_PORT					PORTB

#define SPI0_SCLK_BIT					BIT5
#define SPI0_SCLK_DDR					DDRB
#define SPI0_SCLK_PORT					PORTB

#define	SPI0_MOSI_BIT					BIT3
#define SPI0_MOSI_DDR					DDRB
#define SPI0_MOSI_PORT					PORTB

#define	SPI0_MISO_BIT					BIT4
#define SPI0_MISO_DDR					DDRB
#define SPI0_MISO_PORT					PORTB


#define SPI0_WaitForReceive()
#define SPI0_RxData()	 				(SPDR)

#define SPI0_TxData(Data)				(SPDR = Data)
#define SPI0_WaitForSend()				while( (SPSR & 0x80)==0x00 )

#define SPI0_SendByte(Data)				SPI0_TxData(Data);SPI0_WaitForSend()
#define SPI0_RecvBute()					SPI0_RxData()

// PB4(MISO), PB3(MOSI), PB5(SCK), PB2(/SS)         // CS=1, waiting for SPI start // SPI mode 0, 8MHz
#ifdef USE_DIG8_INTR
#define SPI0_Init()						DDRB  |= SPI0_SS_BIT|SPI0_SCLK_BIT|SPI0_MOSI_BIT|LEDConn_BIT;\
										DDRB  &= ~(SPI0_MISO_BIT|ZG2100_INTR);\
										PORTB = SPI0_SS_BIT;\
										SPCR  = 0x50;\
										SPSR  = 0x01
#else
#define SPI0_Init()						DDRB  |= SPI0_SS_BIT|SPI0_SCLK_BIT|SPI0_MOSI_BIT|LEDConn_BIT;\
										DDRB  &= ~SPI0_MISO_BIT;\
										PORTB = SPI0_SS_BIT;\
										SPCR  = 0x50;\
										SPSR  = 0x01
#endif

//ZG2100 SPI HAL
#define ZG2100_SpiInit					SPI0_Init
#define ZG2100_SpiSendData				SPI0_SendByte
#define ZG2100_SpiRecvData				SPI0_RxData


#define ZG2100_CS_BIT					BIT2
#define ZG2100_CS_DDR					DDRB
#define ZG2100_CS_PORT					PORTB

#define ZG2100_CSInit()					(ZG2100_CS_DDR |= ZG2100_CS_BIT)
#define ZG2100_CSon()					(ZG2100_CS_PORT |= ZG2100_CS_BIT)
#define ZG2100_CSoff()					(ZG2100_CS_PORT &= ~ZG2100_CS_BIT)

#define LEDConn_BIT					BIT1
#define LEDConn_DDR					DDRB
#define LEDConn_PORT				PORTB

#define LED0_BIT					BIT0
#define LED0_DDR					DDRC
#define LED0_PORT					PORTC

#define LED1_BIT					BIT1
#define LED1_DDR					DDRC
#define LED1_PORT					PORTC

#define LED2_BIT					BIT2
#define LED2_DDR					DDRC
#define LED2_PORT					PORTC

#define LED3_BIT					BIT3
#define LED3_DDR					DDRC
#define LED3_PORT					PORTC

#define LED_Init()    (DDRC |= LED0_BIT | LED1_BIT | LED2_BIT | LED3_BIT)

#define LEDConn_on()	(LEDConn_PORT |= LEDConn_BIT)
#define LED0_on()		(LED0_PORT |= LED0_BIT)
#define LED1_on()		(LED0_PORT |= LED1_BIT)
#define LED2_on()		(LED0_PORT |= LED2_BIT)
#define LED3_on()		(LED0_PORT |= LED3_BIT)

#define LEDConn_off()	(LEDConn_PORT &= ~LEDConn_BIT)
#define LED0_off()		(LED0_PORT &= ~LED0_BIT)
#define LED1_off()		(LED0_PORT &= ~LED1_BIT)
#define LED2_off()		(LED0_PORT &= ~LED2_BIT)
#define LED3_off()		(LED0_PORT &= ~LED3_BIT)

#define LED0_toggle()	((LED0_PORT & LED0_BIT)?(LED0_PORT &= ~LED0_BIT):(LED0_PORT |= LED0_BIT))
#define LED1_toggle()	((LED0_PORT & LED1_BIT)?(LED0_PORT &= ~LED1_BIT):(LED0_PORT |= LED1_BIT))
#define LED3_toggle()	((LED0_PORT & LED3_BIT)?(LED0_PORT &= ~LED3_BIT):(LED0_PORT |= LED3_BIT))

#endif /* SPI_H_ */
