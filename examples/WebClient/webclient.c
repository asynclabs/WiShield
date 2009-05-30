
/******************************************************************************

  Filename:		webclient.c
  Description:	Webclient app for the WiShield 1.0

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

#include "uip.h"
#include "uiplib.h"
#include "webclient.h"
#include "config.h"

#include <string.h>

#define WEBCLIENT_TIMEOUT 100

#define WEBCLIENT_STATE_STATUSLINE 0
#define WEBCLIENT_STATE_HEADERS    1
#define WEBCLIENT_STATE_DATA       2
#define WEBCLIENT_STATE_CLOSE      3

#define HTTPFLAG_NONE   0
#define HTTPFLAG_OK     1
#define HTTPFLAG_MOVED  2
#define HTTPFLAG_ERROR  3


#define ISO_nl       0x0a
#define ISO_cr       0x0d

static struct webclient_state s;

const char http_11[9] =
	/* "HTTP/1.1" */
{0x48, 0x54, 0x54, 0x50, 0x2f, 0x31, 0x2e, 0x31, 0 };
const char http_200[5] =
	/* "200 " */
{0x32, 0x30, 0x30, 0x20, 0 };
const char http_301[5] =
	/* "301 " */
{0x33, 0x30, 0x31, 0x20, 0 };
const char http_302[5] =
	/* "302 " */
{0x33, 0x30, 0x32, 0x20, 0 };

/*-----------------------------------------------------------------------------------*/
char* webclient_mimetype(void)
{
	return s.mimetype;
}
/*-----------------------------------------------------------------------------------*/
char* webclient_filename(void)
{
	return s.file;
}
/*-----------------------------------------------------------------------------------*/
char* webclient_hostname(void)
{
	return s.host;
}
/*-----------------------------------------------------------------------------------*/
unsigned short webclient_port(void)
{
	return s.port;
}
/*-----------------------------------------------------------------------------------*/
void webclient_init(void)
{

}
/*-----------------------------------------------------------------------------------*/
static void init_connection(void)
{
	s.state = WEBCLIENT_STATE_STATUSLINE;

	// set the length of the client string to be transmitted
	s.getrequestleft = strlen_P(twitter);

	s.getrequestptr = 0;

	s.httpheaderlineptr = 0;
}
/*-----------------------------------------------------------------------------------*/
void webclient_close(void)
{
	s.state = WEBCLIENT_STATE_CLOSE;
}
/*-----------------------------------------------------------------------------------*/
unsigned char webclient_get(char *host, u16_t port, char *file)
{
	struct uip_conn *conn;
	uip_ipaddr_t ipaddr;

	uip_ipaddr(&ipaddr, host[0],host[1],host[2],host[3]);

	conn = uip_connect(&ipaddr, htons(port));

	if(conn == NULL) {
		return 0;
	}

	s.port = port;
	strncpy(s.file, file, sizeof(s.file));
	strncpy(s.host, host, sizeof(s.host));

	init_connection();
	return 1;
}
/*-----------------------------------------------------------------------------------*/
static unsigned char* copy_string(unsigned char *dest, const unsigned char *src, unsigned char len)
{
	strncpy(dest, src, len);
	return dest + len;
}
/*-----------------------------------------------------------------------------------*/
static void senddata(void)
{
	u16_t len;
	char *getrequest;
	char *cptr;

	if(s.getrequestleft > 0) {
		cptr = getrequest = (char *)uip_appdata;

#if 0
		cptr = copy_string(cptr, http_get, sizeof(http_get) - 1);
		cptr = copy_string(cptr, s.file, strlen(s.file));
		*cptr++ = ISO_space;
		cptr = copy_string(cptr, http_10, sizeof(http_10) - 1);

		cptr = copy_string(cptr, http_crnl, sizeof(http_crnl) - 1);

		cptr = copy_string(cptr, http_host, sizeof(http_host) - 1);
		cptr = copy_string(cptr, s.host, strlen(s.host));
		cptr = copy_string(cptr, http_crnl, sizeof(http_crnl) - 1);

		cptr = copy_string(cptr, http_user_agent_fields,
				strlen(http_user_agent_fields));
#endif

		// copy the client transmit string into the TX buffer
		memcpy_P(cptr, twitter, strlen_P(twitter));

		len = s.getrequestleft > uip_mss()?uip_mss():s.getrequestleft;
		uip_send(&(getrequest[s.getrequestptr]), len);
	}
}
/*-----------------------------------------------------------------------------------*/
static void acked(void)
{
	u16_t len;

	if(s.getrequestleft > 0) {
		len = s.getrequestleft > uip_mss()?uip_mss():s.getrequestleft;
		s.getrequestleft -= len;
		s.getrequestptr += len;
	}
}
/*-----------------------------------------------------------------------------------*/
static u16_t parse_statusline(u16_t len)
{
	char *cptr;
	char* temp;

	while(len > 0 && s.httpheaderlineptr < sizeof(s.httpheaderline)) {
		s.httpheaderline[s.httpheaderlineptr] = *(char *)uip_appdata;
		//((char *)uip_appdata)++;
		temp = (char *)uip_appdata;
		temp++;
		uip_appdata = temp;
		--len;
		if(s.httpheaderline[s.httpheaderlineptr] == ISO_nl) {

			if((strncmp(s.httpheaderline, http_11,sizeof(http_11) - 1) == 0)) {
				cptr = &(s.httpheaderline[9]);
				s.httpflag = HTTPFLAG_NONE;
				if(strncmp(cptr, http_200, sizeof(http_200) - 1) == 0) {
					/* 200 OK */
					s.httpflag = HTTPFLAG_OK;
				}
				else if(strncmp(cptr, http_301, sizeof(http_301) - 1) == 0 ||
						strncmp(cptr, http_302, sizeof(http_302) - 1) == 0) {
					/* 301 Moved permanently or 302 Found. Location: header line
					 * will contain the new location. */
					s.httpflag = HTTPFLAG_MOVED;
				}
				else {
					s.httpheaderline[s.httpheaderlineptr - 1] = 0;
				}
			}
			else {
				uip_abort();
				webclient_aborted();
				return 0;
			}

			/* We're done parsing the status line, so we reset the pointer
			 * and start parsing the HTTP headers.*/
			s.httpheaderlineptr = 0;
			s.state = WEBCLIENT_STATE_HEADERS;
			break;
		}
		else {
			++s.httpheaderlineptr;
		}
	}
	return len;
}
/*-----------------------------------------------------------------------------------*/
static u16_t parse_headers(u16_t len)
{
	char *cptr;
	static unsigned char i;
	char* temp;

	while(len > 0 && s.httpheaderlineptr < sizeof(s.httpheaderline)) {
		s.httpheaderline[s.httpheaderlineptr] = *(char *)uip_appdata;
		//++((char *)uip_appdata);
		temp = (char *)uip_appdata;
		temp++;
		uip_appdata = temp;
		--len;
		if(s.httpheaderline[s.httpheaderlineptr] == ISO_nl) {
			/* We have an entire HTTP header line in s.httpheaderline, so
			 * we parse it. */
			if(s.httpheaderline[0] == ISO_cr) {
				/* This was the last header line (i.e., and empty "\r\n"), so
				 * we are done with the headers and proceed with the actual
				 * data. */
				s.state = WEBCLIENT_STATE_DATA;
				return len;
			}

			/* We're done parsing, so we reset the pointer and start the
			 * next line. */
			s.httpheaderlineptr = 0;
		}
		else {
			++s.httpheaderlineptr;
		}
	}

	return len;
}
/*-----------------------------------------------------------------------------------*/
static void newdata(void)
{
	u16_t len;

	len = uip_datalen();

	if(s.state == WEBCLIENT_STATE_STATUSLINE) {
		len = parse_statusline(len);
	}

	if(s.state == WEBCLIENT_STATE_HEADERS && len > 0) {
		len = parse_headers(len);
	}

	if(len > 0 && s.state == WEBCLIENT_STATE_DATA &&
			s.httpflag != HTTPFLAG_MOVED) {
		webclient_datahandler((char *)uip_appdata, len);
	}
}
/*-----------------------------------------------------------------------------------*/
void webclient_appcall(void)
{
	if(uip_connected()) {
		s.timer = 0;
		s.state = WEBCLIENT_STATE_STATUSLINE;
		senddata();
		webclient_connected();
		return;
	}

	if(s.state == WEBCLIENT_STATE_CLOSE) {
		webclient_closed();
		uip_abort();
		return;
	}

	if(uip_aborted()) {
		webclient_aborted();
	}

	if(uip_timedout()) {
		webclient_timedout();
	}

	if(uip_acked()) {
		s.timer = 0;
		acked();
	}

	if(uip_newdata()) {
		s.timer = 0;
		newdata();
	}

	if(uip_rexmit() || uip_newdata() || uip_acked()) {
		senddata();
	}
	else if(uip_poll()) {
		++s.timer;
		if(s.timer == WEBCLIENT_TIMEOUT) {
			webclient_timedout();
			uip_abort();
			return;
		}
	}

	if(uip_closed()) {
		if(s.httpflag != HTTPFLAG_MOVED) {
			/* Send NULL data to signal EOF. */
			webclient_datahandler(NULL, 0);
		}
		else {
			//webclient_get(s.host, s.port, s.file);
		}
	}
}
/*-----------------------------------------------------------------------------------*/

void webclient_datahandler(char *data, u16_t len)
{
}

void webclient_connected(void)
{
}

void webclient_timedout(void)
{
}

void webclient_aborted(void)
{
}

void webclient_closed(void)
{
}

/** @} */
/** @} */
