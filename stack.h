
/******************************************************************************

  Filename:		stack.h
  Description:	Simple TCP/IP stack for use with WiShield 1.0

 ******************************************************************************

  TCP/IP stack and driver for the WiShield 1.0 wireless devices

  Copyright(c) 2009 Async Labs Inc. All rights reserved.

  The TCP/IP stack is influenced by
  the uIP stack (c) Adam Dunkels <adam@dunkels.com>.

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

******************************************************************************/

#ifndef STACK_H_
#define STACK_H_

typedef struct _ip_conn {
	U32 ripaddr;
	U32 rcv_nxt;
	U32 snd_nxt;
	U16 lport;
	U16 rport;
	U16 len;          /* Length of the data that was previously sent. */
	U16 mss;          /* Current maximum segment size for the connection. */
	U8 state;
}ip_conn_t;

#define SOCK_STREAM		0x01	// TCP

#define MAX_SOCK_NUM	1

#define SOCK_CLOSED				0x00
#define SOCK_LISTEN				0x01
#define SOCK_SYNRECV		   	0x02
#define SOCK_ESTABLISHED		0x03
#define SOCK_LAST_ACK			0x04
#define SOCK_FIN_WAIT			0x05

#define ICMP_ECHO_REPLY 0
#define ICMP_ECHO       8

#define TCP_OPT_MSS     2   /* Maximum segment size TCP option */
#define TCP_OPT_MSS_LEN 4   /* Length of TCP MSS option. */

#define TCP_MSS     (500 - 14 - 40)		// FIXME : defines

#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PSH 0x08
#define TCP_ACK 0x10
#define TCP_URG 0x20
#define TCP_CTL 0x3f

#define IP_HEADER_LEN    20
#define TCP_HEADER_LEN   20
#define IP_TCP_HEADER_LEN (IP_HEADER_LEN + TCP_HEADER_LEN)

#define FLAG_ACKED				1	// outstanding data acked, application can send data
#define FLAG_DATA_AVAILABLE		2	// new data available
#define FLAG_REMOTE_CLOSE		3	// remote host closed connection

#define APP_FLAG_CLOSE		1

#define STACK_ST_IDLE				0
#define STACK_ST_DROP				1
#define STACK_ST_TCP				2
#define STACK_ST_ICMP				3
#define STACK_ST_SEND				4
#define STACK_ST_FOUND				5
#define STACK_ST_RESET				6
#define STACK_ST_FOUND_LISTEN		7
#define STACK_ST_UDP				8
#define STACK_ST_TCP_SEND_NO_CONN	9
#define STACK_ST_SEND_NO_LEN		10
#define STACK_ST_TCP_SEND_SYN		11
#define STACK_ST_TCP_SEND			12
#define STACK_ST_TCP_SEND_ACK		13
#define STACK_ST_APP_SEND			14
#define STACK_ST_TCP_SEND_NO_OPTS	15
#define STACK_ST_IP					16
#define STACK_ST_ARP				17
#define STACK_ST_DONE				18
#define STACK_ST_TCP_SEND_NO_DATA	19

typedef struct _ARPPacket
{
	U16 hardware;
	U16 protocol;
	U8 hwAddrLen;
	U8 protoAddrLen;
	U16 operation;
	U8 senderAddr[6];
	U32 senderIPAddr;
	U8 targetAddr[6];
	U32 targetIPAddr;
}tARPPacket;

typedef struct _ICMPPacket
{
	U8 type;
	U8 code;
	U16 chkSum;
	U8 payload;
}tICMPPacket;

typedef struct _TCPPseudoHeader
{
	U32 srcIPAddr;
	U32 destIPAddr;
	U8 zeros;
	U8 protocol;
	U16 len;
}tTCPPseudoHdr;

typedef struct _TCPPacket
{
	U16 srcPort;
        U16 dstPort;
        U32 seqNum;
        U32 ackNum;
        U8 dataOffset;
        U8 flags;
        U16 winSize;
        U16 chksum;
        U16 urgPtr;
	U8 payload;
}tTCPPacket;

typedef struct _IPPacket
{
	U8 versionHdrLen;
	U8 tos;
	U16 totalLen;
	U16 id;
	U16 flagsOffset;
	U8 ttl;
	U8 protocol;
	U16 hdrChkSum;
	U32 srcIPAddr;
	U32 destIPAddr;
	U8 payload;
}tIPPacket;

#define IPBUF ((tIPPacket*)&GBLBUF[14])
#define ICMPBUF ((tICMPPacket*)&IPBUF->payload)
#define TCPBUF ((tTCPPacket*)&IPBUF->payload)
#define APPBUF ((U8*)&TCPBUF->payload)

U8 socket(U8 protocol, U16 port);
U16 stack_app_data();
void stack_set_app_data(U16 len);
void stack_set_app_more_data(U8 flag);
void stack_process();
U8 validate_chksum(U16* pWord, U16 len, U16* pOther, U16 otherLen, U8 odd);
U16 calc_chksum(U16* pWord, U16 len, U16* pOther, U16 otherLen, U8 odd);

#endif /* STACK_H_ */
