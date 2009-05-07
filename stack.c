
/******************************************************************************

  Filename:		stack.c
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

#include <string.h>
#include "types.h"
#include "config.h"
#include "g2100.h"
#include "stack.h"
#include "spi.h"

// The one and only buffer
// used by the driver for management
// used by the stack for all packet processing
// used by the application to send and receive data
U8 buf[500];

static U16 ipid;
static U32 iss;
static tTCPPseudoHdr pseudoTCP;

static ip_conn_t ip_conn[MAX_SOCK_NUM];

static U8 stack_state;
static U16 stack_pkt_len;
static U8 sock_num;
static U16 stack_app_data_len;
static U8 ip_flags;
static U8 app_flags;

U8 socket(U8 protocol, U16 port)
{
	U8 sock;

	for (sock = 0; sock < MAX_SOCK_NUM; sock++) {
		if (ip_conn[sock].state == SOCK_CLOSED) {
			ip_conn[sock].state = SOCK_LISTEN;
			ip_conn[sock].lport = HTONS(port);
			return sock;
		}
	}

	return 255;
}

U8 validate_chksum(U16* pWord, U16 len, U16* pOther, U16 otherLen, U8 odd)
{
	U32 sum = 0;
	U16 i;
	U8 retVal = 0;

	for (i=0; i < len; i++)
	{
		sum += pWord[i];
		while (sum>>16)
			sum = (sum & 0xffff) + (sum >> 16);
	}

	if (odd)
	{
		sum += (pWord[i] & 0xff00);
		while (sum>>16)
			sum = (sum & 0xffff) + (sum >> 16);
	}

	if (otherLen)
		for (i=0; i < otherLen; i++)
		{
			sum += pOther[i];
			while (sum>>16)
				sum = (sum & 0xffff) + (sum >> 16);
		}

	if (sum == 0xffff)
		retVal = 1;

	return retVal;
}

U16 calc_chksum(U16* pWord, U16 len, U16* pOther, U16 otherLen, U8 odd)
{
	U32 sum = 0;
	U16 i;

	for (i=0; i < len; i++)
	{
		sum += pWord[i];
		while (sum>>16)
			sum = (sum & 0xffff) + (sum >> 16);
	}

	if (odd)
	{
		sum += (pWord[i] & 0xff00);
		while (sum>>16)
			sum = (sum & 0xffff) + (sum >> 16);
	}

	if (otherLen)
		for (i=0; i < otherLen; i++)
		{
			sum += pOther[i];
			while (sum>>16)
				sum = (sum & 0xffff) + (sum >> 16);
		}

	return ((U16)~sum);
}

U16 stack_app_data()
{
	return stack_app_data_len;
}

void stack_set_app_data(U16 len)
{
	stack_app_data_len = len;
}

void stack_set_app_more_data(U8 flag)
{
	app_flags = flag;
}

void stack_process()
{
	U16 tmp16;
	U32 tmp32;

	U8 done = 0;

	do {
		switch (stack_state) {
		case STACK_ST_IDLE:
			if (zg_get_rx_status()) {
				U16 protocol = HTONS((U16)*((U16*)&GBLBUF[12]));

				if (protocol == 0x0806) {
					stack_state = STACK_ST_ARP;
					break;
				}

				if (protocol == 0x0800) {
					stack_state = STACK_ST_IP;
					break;
				}
			}

			stack_state = STACK_ST_DROP;
			break;
		case STACK_ST_ARP:
		{
			tARPPacket* pARP = (tARPPacket*)&GBLBUF[14];

			if (pARP->operation == HTONS(1) && pARP->targetIPAddr == local_ip)
			{
				// build ARP Response
				memcpy(&GBLBUF[0], &GBLBUF[6], 6);
				memcpy(&GBLBUF[6], zg_get_mac(), 6);
				pARP->operation = HSTOZGS(2);
				memcpy(pARP->targetAddr, pARP->senderAddr, 6);
				pARP->targetIPAddr = pARP->senderIPAddr;
				memcpy(pARP->senderAddr, zg_get_mac(), 6);
				pARP->senderIPAddr = local_ip;

				// send ARP Response
				zg_set_buf(GBLBUF, 42);
				zg_set_tx_status(1);
			}

			stack_state = STACK_ST_DONE;
			break;
		}
		case STACK_ST_IP:
			// IP version check
			if(IPBUF->versionHdrLen != 0x45) {
				stack_state = STACK_ST_DROP;
				break;
			}

			// We cannot handle fragmented packets
			if((IPBUF->flagsOffset & HTONS(0x2000)) != 0) {
				// fragmented packets not supported
				stack_state = STACK_ST_DROP;
				break;
			}

			// Verify IP address
			// FIXME : need to check for broadcast too
			if(IPBUF->destIPAddr != local_ip) {
				stack_state = STACK_ST_DROP;
				break;
			}

			// Validate the IP checksum
			if(!validate_chksum((U16*)IPBUF, (IPBUF->versionHdrLen & 0x0f)*2, NULL, 0, 0)) { /* Compute and check the IP header checksum. */
				stack_state = STACK_ST_DROP;
				break;
			}

			// Is this a TCP packet ?
			if(IPBUF->protocol == 0x06) {
				stack_state = STACK_ST_TCP;
				break;
			}

			// Is this a UDP packet ?
			if(IPBUF->protocol == 0x11) {
				// UDP support
				stack_state = STACK_ST_UDP;
				break;
			}

			/* ICMPv4 packet */
			if(IPBUF->protocol == 0x01) {
				stack_state = STACK_ST_ICMP;
				break;
			}

			stack_state = STACK_ST_DROP;
			break;
		case STACK_ST_ICMP:
			// process ICMP
			if(ICMPBUF->type != ICMP_ECHO) {
				stack_state = STACK_ST_DROP;
				break;
			}

			ICMPBUF->type = ICMP_ECHO_REPLY;

			if(ICMPBUF->chkSum >= HTONS(0xffff - (ICMP_ECHO << 8))) {
				ICMPBUF->chkSum += HTONS(ICMP_ECHO << 8) + 1;
			} else {
				ICMPBUF->chkSum += HTONS(ICMP_ECHO << 8);
			}

			IPBUF->destIPAddr = IPBUF->srcIPAddr;
			IPBUF->srcIPAddr = local_ip;

			stack_pkt_len = ZGSTOHS(IPBUF->totalLen);

			stack_state = STACK_ST_SEND;
			break;
		case STACK_ST_UDP:
			// not supported
			stack_state = STACK_ST_DROP;
			break;
		case STACK_ST_TCP:
		{
			// process TCP
			U8 i;

			pseudoTCP.srcIPAddr = IPBUF->srcIPAddr;
			pseudoTCP.destIPAddr = IPBUF->destIPAddr;
			pseudoTCP.zeros = 0;
			pseudoTCP.protocol = 6;
			pseudoTCP.len = HSTOZGS(ZGSTOHS(IPBUF->totalLen) - ((IPBUF->versionHdrLen & 0x0f)*4));

			// Validate TCP checksum
			//if(!validate_chksum((U16*)TCPBUF, (ZGSTOHS(IPBUF->totalLen) - ((IPBUF->versionHdrLen & 0x0f)*4))/2, (U16*)&pseudoTCP, 6, (ZGSTOHS(pseudoTCP.len) & 0x0001)?1:0)) {
			//	stack_state = STACK_ST_DROP;
			//	break;
			//}

			// Scan for active connections
			for(i = 0; i < MAX_SOCK_NUM; i++) {
				if (	ip_conn[i].state != SOCK_CLOSED &&
						ip_conn[i].lport == TCPBUF->dstPort &&
						ip_conn[i].rport == TCPBUF->srcPort &&
						ip_conn[i].ripaddr == IPBUF->srcIPAddr) {
					sock_num = i;
					stack_state = STACK_ST_FOUND;
					break;
				}
			}

			if (stack_state != STACK_ST_TCP) {
				break;
			}

			if((TCPBUF->flags & TCP_CTL) != TCP_SYN) {
				stack_state = STACK_ST_RESET;
				break;
			}

			// Are there servers listening ?
			for(i = 0; i < MAX_SOCK_NUM; i++) {
				if (ip_conn[i].state == SOCK_LISTEN && ip_conn[i].lport == TCPBUF->dstPort) {
					sock_num = i;
					stack_state = STACK_ST_FOUND_LISTEN;
					break;
				}
			}

			// send reset
			if (stack_state == STACK_ST_TCP) {
				stack_state = STACK_ST_RESET;
			}
			break;
		}
		case STACK_ST_RESET:
			// send reset packet
			if(TCPBUF->flags & TCP_RST) {
				stack_state = STACK_ST_DROP;
				break;
			}

			TCPBUF->flags = TCP_RST | TCP_ACK;
			TCPBUF->dataOffset = 5 << 4;

			tmp32 = TCPBUF->seqNum;
			TCPBUF->seqNum = TCPBUF->ackNum;
			TCPBUF->ackNum = tmp32;

			tmp32 = HTOZGL(TCPBUF->ackNum);
			tmp32++;
			TCPBUF->ackNum = HTOZGL(tmp32);

			tmp16 = TCPBUF->srcPort;
			TCPBUF->srcPort = TCPBUF->dstPort;
			TCPBUF->dstPort = tmp16;

			IPBUF->destIPAddr = IPBUF->srcIPAddr;
			IPBUF->srcIPAddr = local_ip;

			stack_pkt_len = 20+20;

			stack_state = STACK_ST_TCP_SEND_NO_CONN;
			break;
		case STACK_ST_TCP_SEND_ACK:
			TCPBUF->flags = TCP_ACK;
		case STACK_ST_TCP_SEND_NO_DATA:
			stack_pkt_len = IP_TCP_HEADER_LEN;
		case STACK_ST_TCP_SEND_NO_OPTS:
			TCPBUF->dataOffset = (TCP_HEADER_LEN / 4) << 4;
		case STACK_ST_TCP_SEND:
			// Fill TCP + IP headers, calculate checksum and send
			TCPBUF->ackNum = ip_conn[sock_num].rcv_nxt;

			TCPBUF->seqNum = ip_conn[sock_num].snd_nxt;

			IPBUF->protocol = 0x06;

			TCPBUF->srcPort  = ip_conn[sock_num].lport;
			TCPBUF->dstPort = ip_conn[sock_num].rport;

			IPBUF->srcIPAddr = local_ip;
			IPBUF->destIPAddr = ip_conn[sock_num].ripaddr;

			TCPBUF->winSize = HTONS(TCP_MSS);
		case STACK_ST_TCP_SEND_NO_CONN:
			IPBUF->ttl = 64;
			IPBUF->totalLen = HTONS(stack_pkt_len);
			TCPBUF->urgPtr = 0;
			TCPBUF->chksum = 0;
			IPBUF->hdrChkSum = 0;

			pseudoTCP.len = HSTOZGS(ZGSTOHS(IPBUF->totalLen) - ((IPBUF->versionHdrLen & 0x0f)*4));

			// Calculate TCP checksum
			TCPBUF->chksum = calc_chksum((U16*)TCPBUF, (ZGSTOHS(IPBUF->totalLen) - ((IPBUF->versionHdrLen & 0x0f)*4))/2, (U16*)&pseudoTCP, 6, (ZGSTOHS(pseudoTCP.len) & 0x0001)?1:0);
		case STACK_ST_SEND_NO_LEN:
			IPBUF->versionHdrLen = 0x45;
			IPBUF->tos = 0;
			IPBUF->flagsOffset = 0;
			++ipid;
			IPBUF->id = HTONS(ipid);

			// Calculate IP checksum
			IPBUF->hdrChkSum = calc_chksum((U16*)IPBUF, (IPBUF->versionHdrLen & 0x0f)*2, NULL, 0, 0);
		case STACK_ST_SEND:
			// packet processing done
			// flip src and dst MAC address
			memcpy(&GBLBUF[0], &GBLBUF[6], 6);
			memcpy(&GBLBUF[6], zg_get_mac(), 6);
			stack_pkt_len += 14;
			zg_set_buf(GBLBUF, stack_pkt_len);
			zg_set_tx_status(1);

			stack_state = STACK_ST_DONE;
			break;
		case STACK_ST_FOUND_LISTEN:
			// Found listening servers
			ip_conn[sock_num].rport = TCPBUF->srcPort;
			ip_conn[sock_num].ripaddr = IPBUF->srcIPAddr;
			ip_conn[sock_num].state = SOCK_SYNRECV;
			ip_conn[sock_num].snd_nxt = HTOZGL(iss);
			ip_conn[sock_num].len = 1;

			tmp32 = HTOZGL(TCPBUF->seqNum);
			tmp32++;
			ip_conn[sock_num].rcv_nxt = HTOZGL(tmp32);

			TCPBUF->flags = TCP_ACK;
		case STACK_ST_TCP_SEND_SYN:
			TCPBUF->flags |= TCP_SYN;

			// Send TCP MSS
			TCPBUF->payload = TCP_OPT_MSS;
			((U8*)&(TCPBUF->payload))[1] = TCP_OPT_MSS_LEN;
			((U8*)&(TCPBUF->payload))[2] = (((U16)TCP_MSS)&0xff00)>>8;
			((U8*)&(TCPBUF->payload))[3] = ((U16)TCP_MSS)&0x00ff;
			TCPBUF->dataOffset = ((20 + TCP_OPT_MSS_LEN) / 4) << 4;

			stack_pkt_len = 40 + 4;	// IP_TCP_HEADER_LEN + TCP_OPT_MSS_LEN

			stack_state = STACK_ST_TCP_SEND;
			break;
		case STACK_ST_FOUND:
			// process connection
			if(TCPBUF->flags & TCP_RST) {
				ip_conn[sock_num].state = SOCK_CLOSED;

				// FIXME :	reset all other parameters.
				//			go back to listening if configured as server

				stack_state = STACK_ST_DROP;
				break;
			}

			stack_app_data_len = ZGSTOHS(IPBUF->totalLen);
			stack_app_data_len -= ((IPBUF->versionHdrLen & 0x0f)*4);
			stack_app_data_len -= (((TCPBUF->dataOffset & 0xf0)>>4)*4);

			/* Check if the incoming segment acknowledges any outstanding data.
			 * If so, we update the sequence number, reset the length of the
			 * outstanding data */
			if((TCPBUF->flags & TCP_ACK) && ip_conn[sock_num].len) {
				tmp32 = HTOZGL(ip_conn[sock_num].snd_nxt);
				tmp32 = tmp32 + ip_conn[sock_num].len;

				if (TCPBUF->ackNum == HTOZGL(tmp32)) {
					// Update sequence number
					ip_conn[sock_num].snd_nxt = HTOZGL(tmp32);

					// Reset length of outstanding data
					ip_conn[sock_num].len = 0;

					ip_flags = FLAG_ACKED;
				}

			}

			switch(ip_conn[sock_num].state) {
			case SOCK_SYNRECV:
				/* In SYN_RCVD we have sent out a SYNACK in response to a SYN,
				 * and we are waiting for an ACK that acknowledges the data
				 * we sent out the last time. */
				if(ip_conn[sock_num].len == 0) {
					ip_conn[sock_num].state = SOCK_ESTABLISHED;
					ip_conn[sock_num].len = 0;
					if(stack_app_data_len > 0) {
						ip_flags |= FLAG_DATA_AVAILABLE;

						tmp32 = HTOZGL(ip_conn[sock_num].rcv_nxt);
						tmp32 = tmp32 + stack_app_data_len;
						ip_conn[sock_num].rcv_nxt = HTOZGL(tmp32);

						stack_state = STACK_ST_APP_SEND;
						done = 1;
					}
					else {
						stack_state = STACK_ST_DROP;
					}

					break;
				}
				else {
					stack_state = STACK_ST_TCP_SEND_ACK;
				}
				break;
			case SOCK_ESTABLISHED:
				/* In the ESTABLISHED state, we call upon the application to feed
			    data into the uip_buf. If the FLAG_ACKED flag is set, the
			    application should put new data into the buffer, otherwise we are
			    retransmitting an old segment, and the application should put that
			    data into the buffer.

			    If the incoming packet is a FIN, we should close the connection on
			    this side as well, and we send out a FIN and enter the LAST_ACK
			    state. We require that there is no outstanding data; otherwise the
			    sequence numbers will be screwed up. */

				if(TCPBUF->flags & TCP_FIN) {
					if(ip_conn[sock_num].len) {
						stack_state = STACK_ST_DROP;
						break;
					}

					tmp32 = HTOZGL(ip_conn[sock_num].rcv_nxt);
					tmp32 = tmp32 + 1 + stack_app_data_len;
					ip_conn[sock_num].rcv_nxt = HTOZGL(tmp32);

					ip_flags |= FLAG_REMOTE_CLOSE;

					ip_conn[sock_num].len = 1;
					ip_conn[sock_num].state = SOCK_LAST_ACK;

					TCPBUF->flags = TCP_FIN | TCP_ACK;

					stack_state = STACK_ST_TCP_SEND_NO_DATA;
					break;
				}

				/* If stack_app_data_len > 0 we have TCP data in the packet,
				 * and we flag this by setting the FLAG_DATA_AVAILABLE flag and
				 * update the sequence number we acknowledge. */
				if(stack_app_data_len > 0) {
					ip_flags |= FLAG_DATA_AVAILABLE;
					tmp32 = HTOZGL(ip_conn[sock_num].rcv_nxt);
					tmp32 = tmp32 + stack_app_data_len;
					ip_conn[sock_num].rcv_nxt = HTOZGL(tmp32);
				}

				if(ip_flags & (FLAG_DATA_AVAILABLE | FLAG_ACKED)) {
					stack_state = STACK_ST_APP_SEND;
					done = 1;
					break;
				}

				stack_state = STACK_ST_DROP;
				break;
			case SOCK_LAST_ACK:
				/* We can close this connection if the peer has acknowledged
				 * our FIN. This is indicated by the FLAG_ACKED flag. */
				if(ip_flags & FLAG_ACKED) {
					ip_conn[sock_num].state = SOCK_LISTEN;
					ip_conn[sock_num].len = 0;
					ip_conn[sock_num].mss = 0;
					ip_conn[sock_num].rcv_nxt = 0;
					ip_conn[sock_num].ripaddr = 0;
					ip_conn[sock_num].rport = 0;
					ip_conn[sock_num].snd_nxt = 0;
				}
				break;
			case SOCK_FIN_WAIT:
				if(stack_app_data_len > 0) {
					tmp32 = HTOZGL(ip_conn[sock_num].rcv_nxt);
					tmp32 = tmp32 + stack_pkt_len;
					ip_conn[sock_num].rcv_nxt = HTOZGL(tmp32);
				}

				if(TCPBUF->flags & TCP_FIN) {
					tmp32 = HTOZGL(ip_conn[sock_num].rcv_nxt);
					tmp32 = tmp32 + 1;
					ip_conn[sock_num].rcv_nxt = HTOZGL(tmp32);

					ip_conn[sock_num].state = SOCK_LISTEN;

					stack_state = STACK_ST_TCP_SEND_ACK;
					break;
				}
			default:
				break;
			}

			if (stack_state == STACK_ST_FOUND) {
				stack_state = STACK_ST_DROP;
			}
			break;
		case STACK_ST_APP_SEND:
			// Application has data to be sent
			if(stack_app_data_len > 0) {
				if(ip_conn[sock_num].len == 0) {
			    	/* Remember how much data we send out now so that we
			     	 * know when everything has been acknowledged. */
			        ip_conn[sock_num].len = stack_app_data_len;
			        stack_app_data_len = 0;
			        stack_pkt_len = ip_conn[sock_num].len + IP_TCP_HEADER_LEN;
			        TCPBUF->flags = TCP_ACK | TCP_PSH | TCP_FIN;

			        ip_conn[sock_num].state = SOCK_FIN_WAIT;

			        stack_state = STACK_ST_TCP_SEND_NO_OPTS;
			        break;
			     }
			}
			else {
				/* If the app has no data to send, just send out a pure ACK
				 * if there was new data. */
				if(ip_flags & FLAG_DATA_AVAILABLE) {
					stack_pkt_len = IP_TCP_HEADER_LEN;
					TCPBUF->flags = TCP_ACK;
					stack_state = STACK_ST_TCP_SEND_NO_OPTS;
					break;
				}

				if(app_flags & APP_FLAG_CLOSE) {
					ip_conn[sock_num].len = 1;
					ip_conn[sock_num].state = SOCK_FIN_WAIT;
					TCPBUF->flags = TCP_FIN | TCP_ACK;
					stack_state = STACK_ST_TCP_SEND_NO_DATA;
					break;
				}
			}

			stack_state = STACK_ST_DROP;
			break;
		case STACK_ST_DROP:
			// drop IP packet
		case STACK_ST_DONE:
		default:
			stack_state = STACK_ST_IDLE;
			zg_clear_rx_status();
			done = 1;
			break;
		}
	} while (!done);
}
