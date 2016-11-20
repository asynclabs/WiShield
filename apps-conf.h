
/******************************************************************************

  Filename:		apps-conf.h
  Description:	Web application configuration file

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

#ifndef __APPS_CONF_H__
#define __APPS_CONF_H__

//Here we include the header file for the application(s) we use in our project.
//#define APP_WEBSERVER
//#define APP_WEBCLIENT
//#define APP_SOCKAPP
//#define APP_UDPAPP
#define APP_WISERVER

#ifdef APP_WEBSERVER
#include "webserver.h"
#endif

#ifdef APP_WEBCLIENT
#include "webclient.h"
#endif

#ifdef APP_SOCKAPP
#include "socketapp.h"
#endif

#ifdef APP_UDPAPP
#include "udpapp.h"
#endif

#ifdef APP_WISERVER
#include "server.h"
#endif

#endif /*__APPS_CONF_H__*/
