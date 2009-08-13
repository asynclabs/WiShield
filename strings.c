/******************************************************************************

  Filename:		strings.c
  Description:	Contains program memory strings for the WiServer

 ******************************************************************************

  Copyright(c) 2009 Mark A. Patel  All rights reserved.

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

   Author               Date        Comment
  ---------------------------------------------------------------
   Mark A. Patel		06/10/2009	Initial version

 *****************************************************************************/

#include "server.h"
#include <avr/pgmspace.h>


// This file contains various strings used by the WiServer
// They are stored in program memory to reduce RAM usage.

// HTTP OK response
const prog_char httpOK[] = {"HTTP/1.0 200 OK"};

// HTTP Not Found response
const prog_char httpNotFound[] = {"HTTP/1.0 404 Not Found"};


#ifdef ENABLE_CLIENT_MODE

// Trailing HTTP/1.0 for GET and POST requests
const prog_char http10[] = {" HTTP/1.0"};

// Authorization: Basic field name
const prog_char authBasic[] = {"Authorization: Basic "};

// Content-Length field name
const prog_char contentLength[] = {"Content-Length:     "};

// User agent field with value
const prog_char userAgent[] = {"User-Agent: WiServer/1.1"};

// Content type form data
const prog_char contentTypeForm[] = {"Content-Type: application/x-www-form-urlencoded"};

// POST request
const prog_char post[] = {"POST "};

// GET request
const prog_char get[] = {"GET "};

// Host, User-Agent, and Content-Type lines for Twitter POSTs
const prog_char host[] = {"Host: "};

// Status prefix for Twitter
const prog_char status[] = {"status="};

/* Base64 Encoder data */
const prog_char base64Chars[] PROGMEM = {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};


#endif // ENABLE_CLIENT_MODE

