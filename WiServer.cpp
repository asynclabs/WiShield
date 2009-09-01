/******************************************************************************

  Filename:		WiSever.cpp
  Description:	Main library code for the WiServer library

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
   Mark A. Patel		05/27/2009	Initial version

   Mark A. Patel		06/05/2009	Revised to work with stack version 1.1,
								    plus added various enhancements such as
								    multi-pass transmission, local client
								    checks, activity LED support, etc.


 *****************************************************************************/


#include "WProgram.h"
#include "WiServer.h"

extern "C" {
    #include "g2100.h"
	#include "spi.h"
	#include "uip.h"
    #include "server.h"
	#include "global-conf.h"
	void stack_init(void);
	void stack_process(void);
}

#ifdef APP_WISERVER

#define CR 13
#define LF 10

// Strings stored in program memory (defined in strings.c)
extern const prog_char httpOK[];
extern const prog_char httpNotFound[];
extern const prog_char http10[];
extern const prog_char post[];
extern const prog_char get[];
extern const prog_char authBasic[];
extern const prog_char host[];
extern const prog_char userAgent[];
extern const prog_char contentTypeForm[];
extern const prog_char contentLength[];
extern const prog_char status[];
extern const prog_char base64Chars[];



/* Application's callback function for serving pages */
pageServingFunction callbackFunc;

/* Digital output pin to indicate TX activity */
char txPin = -1;

/* Digital output pin to indicate RX activity */
char rxPin = -1;

/* Enables basic log messages via Serial */
boolean verbose = false;


void Server::init(pageServingFunction function) {

	// WiShield init
	zg_init();

#ifdef USE_DIG0_INTR
	attachInterrupt(0, zg_isr, LOW);
#endif

#ifdef USE_DIG8_INTR
	// set digital pin 8 on Arduino
	// as ZG interrupt pin
	PCICR |= (1<<PCIE0);
	PCMSK0 |= (1<<PCINT0);
#endif

	while(zg_get_conn_state() != 1) {
		zg_drv_process();
	}

	// Start the stack
	stack_init();

	// Store the callback function for serving pages
	// and start listening for connections on port 80 if
	// the function is non-null
	callbackFunc = function;
	if (callbackFunc) {
		// Listen for server requests on port 80
		uip_listen(HTONS(80));
	}

#ifdef DEBUG
	verbose = true;
	Serial.println("WiServer init called");
#endif // DEBUG
}

#ifdef USE_DIG8_INTR
// PCINT0 interrupt vector
ISR(PCINT0_vect)
{
	zg_isr();
}
#endif

void Server::setIndicatorPins(int tx, int rx) {
	// Store the pin numbers
	txPin = tx;
	rxPin = rx;
	// Set pin modes as needed
	if (tx != -1) pinMode(tx, OUTPUT);
	if (rx != -1) pinMode(rx, OUTPUT);
}

/*
 * Sets the TX pin (if enabled) to the specified value (HIGH or LOW)
 */
void setTXPin(byte value) {
	if (txPin != -1) digitalWrite(txPin, value);
}

/*
 * Sets the RX pin (if enabled) to the specified value (HIGH or LOW)
 */
void setRXPin(byte value) {
	if (rxPin != -1) digitalWrite(rxPin, value);
}


void Server::enableVerboseMode(boolean enable) {
    verbose = enable;
}



/******* Generic printing and sending functions ********/


void Server::write_P(const char data[], int len) {
	while (len-- > 0) {
		this->write(pgm_read_byte(data++));
	}
}


void Server::print_P(const char s[]) {
	char c = pgm_read_byte(s);
	while (c) {
		this->print(c);
		c = pgm_read_byte(++s);
	}
}


void Server::println_P(const char c[]) {
	this->print_P(c);
	this->println();
}



void Server::printTime(long t) {

	long secs = t / 1000;
	int mins = (int)(secs / 60);
	int hours = mins / 60;

	hours %= 24;
	this->print(hours / 10);
	this->print(hours % 10);
	this->print(':');

	mins %= 60;
	this->print(mins / 10);
	this->print(mins % 10);
	this->print(':');

	secs %= 60;
	this->print(secs / 10);
	this->print(secs % 10);
}


/*
 * Writes a byte to the virtual buffer for the current connection
 */
void Server::write(uint8_t b) {

	// Make sure there's a current connection
	if (uip_conn) {
		// Check if the cursor is within the range that maps to the uip_appdata buffer
		// (and we'll increment the cursor while we're at it)
		int offset = (int)(uip_conn->appstate.cursor++) - uip_conn->appstate.ackedCount;
		if ((offset >= 0) && (offset < (int)uip_conn->mss)) {
			// Write the byte to the corresponding location in the buffer
			*((char*)uip_appdata + offset) = b;
		}
	}
}


/*
 * Sends the real data in the current connection's virtual buffer
 */
void send() {

	uip_tcp_appstate_t *app = &(uip_conn->appstate);

	// Find the intersection of the virtual buffer and the real uip buffer
	int len = (int)app->cursor - app->ackedCount;
	len = len < 0 ? 0 : len;
	len = len > (int)uip_conn->mss ? (int)uip_conn->mss : len;

	if (verbose) {
		Serial.print("TX ");
		Serial.print(len);
		Serial.println(" bytes");
	}

#ifdef DEBUG
	Serial.print(app->ackedCount);
	Serial.print(" - ");
	Serial.print(app->ackedCount + len - 1);
	Serial.print(" of ");
	Serial.println((int)app->cursor);
#endif // DEBUG

	// Send the real bytes from the virtual buffer and record how many were sent
	uip_send(uip_appdata, len);
	app->sentCount = len;
	setTXPin(HIGH);
}


/******* Server mode functions ********/


/*
 * Processes a line of data in an HTTP request.  This function looks
 * for GET and saves a copy of the URL in the current connection's
 * server request.  It also sets the request's isValid flag if the
 * URL has been saved and an empty line is found.
 */
boolean processLine(char* data, int len) {

	// Check for a valid GET line
	if ((uip_conn->appstate.request == NULL) && (strncmp(data, "GET /", 4) == 0)) {
		// URL starts at the '/'
		char* start = data + 4;
		// Find trailing space after the URL
		data = start;
		char* end = data + len;
		while (++data < end) {
			if (*data == ' ') {
				// Replace the space with a NULL to terminate it
				*(data++) = 0;
				// Compute length of the URL including the NULL
				int len = data - start;
				// Allocate space for the URL and copy the contents
				uip_conn->appstate.request = malloc(len);
				memcpy(uip_conn->appstate.request, start, len);
				return false;
			}
		}
		// No space, not valid
	}

	return (len == 0);
}


/*
 * Processes a packet of data that supposedly contains an HTTP request
 * This function looks for CR/LF (or just LF) and calls processLine
 * with each line of data found.
 */
boolean processPacket(char* data, int len) {

	// Address after the last byte of data
	char* end = data + len;
	// Start of current line
	char* start = data;

	// Scan through the bytes in the packet looking for a Line Feed character
	while (data < end) {
		if (*data == LF) {
			// Determine the length of the line excluding the Line Feed
			int lineLength = data - start;

			if (*(data - 1) == CR) {
				lineLength--;
			}

			*(start + lineLength) = 0;
			// Process the line
			if (processLine(start, lineLength)) {
				return true;
			}
			// Set up for the start of the next line
			start = ++data;
		} else {
			// Go to the next byte
			data++;
		}
	}
	return false;
}


/*
 * Attempts to send the requested page for the current connection
 */
void sendPage() {

	// Reset the virtual buffer cursor
	uip_conn->appstate.cursor = 0;

	// Start off with an HTTP OK response header and a blank line
	WiServer.println_P(httpOK);
	WiServer.println();

	// Call the application's 'sendPage' function and ask it to
	// generate the requested page content.
	if (!callbackFunc((char*)uip_conn->appstate.request)) {
		// The URL is not recognized by the sketch
		// Reset the cursor and overwrite the HTTP OK header with a 404 message
		uip_conn->appstate.cursor = 0;
		WiServer.println_P(httpNotFound);
		WiServer.println();
#ifdef DEBUG
 		Serial.println("URL Not Found");
#endif // DEBUG
	}
	// Send the 'real' bytes in the buffer
	send();
}


boolean Server::sendInProgress() {
	return false; // FIX ME
}


boolean Server::clientIsLocal() {
	// Check if there is a current connection
	if (uip_conn != NULL) {
		// Check if the remote host is local to the server
		uip_ipaddr_t hostaddr, mask;
		// Get the server's address
		uip_gethostaddr(&hostaddr);
		// Get the subnet mask
		uip_getnetmask(&mask);
		// Compare with the client's address
		return uip_ipaddr_maskcmp(&hostaddr, uip_conn->ripaddr, &mask);
	}
	return false;
}

/*
 * Handles high-level server communications
 */
void server_task_impl() {

	// Get the connection's app state
	uip_tcp_appstate_t *app = &(uip_conn->appstate);

	if (uip_connected()) {

		if (verbose) {
			Serial.println("Server connected");
		}

		// Initialize the server request data
		app->ackedCount = 0;
		app->request = NULL;
	}

	if (uip_newdata()) {
 		setRXPin(HIGH);
		// Process the received packet and check if a valid GET request had been received
		if (processPacket((char*)uip_appdata, uip_datalen()) && app->request) {
			if (verbose) {
				Serial.print("Processing request for ");
				Serial.println((char*)app->request);
			}
			sendPage();
		}
	}


	// Did we get an ack for the last packet?
	if (uip_acked()) {
		// Record the bytes that were successfully sent
		app->ackedCount += app->sentCount;
		app->sentCount = 0;

		// Check if we're done or need to send more content for this
		// request
		if (app->ackedCount == (int)app->cursor) {
			// Done with the current request and connection
			uip_close();
		} else {
			// Generate the content again to send the next packet of data
			sendPage();
		}
	}

	// Check if we need to retransmit
	if (uip_rexmit()) {
		// Send the same data again (same ackedCount value)
		sendPage();
	}

	if (uip_aborted() || uip_closed() || uip_timedout()) {

		// Check if a URL was stored for this connection
		if (app->request != NULL) {
			if (verbose) {
				Serial.println("Server connection closed");
			}

			// Free RAM and clear the pointer
			free(app->request);
			app->request = NULL;
		}
	}
}



/******* Client mode functions ********/
#ifdef ENABLE_CLIENT_MODE

/* Linked list of queued client requests */
GETrequest* queue = NULL;

void Server::submitRequest(GETrequest *req) {
	 // Check for an empty queue
	if (queue == NULL) {
		// Point to the new request
		queue = req;
	} else {
		// Find the tail of the queue
		GETrequest* r = queue;
		while (r->next != NULL) {
			r = r->next;
		}
		// Append the new request
		r->next = req;
	}
	// Set the request as being active
	req->active = true;
}


/**
 * Sends the request for the current connection
 */
void sendRequest() {

	uip_tcp_appstate_t *app = &(uip_conn->appstate);
	GETrequest *req = (GETrequest*)app->request;

	// Reset the virtual buffer
	app->cursor = 0;

	// Indicates if this is a POST request (instead of a GET)
	// Main difference is that POST requests have a body and a
	// callback function to generate said body
	bool isPost = req->body != NULL;

	// Write out the request header
	WiServer.print_P(isPost ? post : get);
	WiServer.print(req->URL);
	WiServer.println_P(http10);

	// Host name
	WiServer.print_P(host);
	WiServer.println(req->hostName);

	// Auth data (if applicable)
	if (req->auth) {
		WiServer.print_P(authBasic);
		WiServer.println(req->auth);
	}

	// User agent (WiServer, of course!)
	WiServer.println_P(userAgent);

	if (isPost) {
		// Since a post has a body after the blank header line, it has to include
		// an accurate content length so that the server knows when it has received
		// all of the body data.
		char* lengthFieldPos; // Cursor position where the content length place holder starts
		char* contentStart; // Start of the body
		char* contentEnd; // End of the body

		// Just form data for now
		WiServer.println_P(contentTypeForm);

		// Content length line (with 4-space placeholder for the value)
		WiServer.println_P(contentLength);
		// Make a note of where the place holder is so we can fill it in later
		lengthFieldPos = app->cursor - 6; // 6 bytes for CR, LF, and 4 spaces

		// Blank line to indicate end of header
		WiServer.println();

		// Body starts here
		contentStart = app->cursor;

		// Print the body preamble if the request has one
		if (req->bodyPreamble) {
			WiServer.print(req->bodyPreamble);
		}

		// Have the sketch provide the body for the POST
		req->body();

		// Body ends here
		contentEnd = app->cursor;

		// Move the cursor back to the content length value and write in the real length
		app->cursor = lengthFieldPos;
		WiServer.print((int)(contentEnd - contentStart));

		// Put the cursor back at the end of the body so that all of the data gets sent
		app->cursor = contentEnd;

	} else {
		// Blank line to indicate end of GET header
		WiServer.println();
	}

	// Send the 'real' bytes in the buffer
	send();
}

/**
 * Handle client communications
 */
void client_task_impl() {

	uip_tcp_appstate_t *app = &(uip_conn->appstate);
	GETrequest *req = (GETrequest*)app->request;

	if (uip_connected()) {

		if (verbose) {
			Serial.print("Connected to ");
			Serial.println(req->hostName);
		}
		app->ackedCount = 0;
		sendRequest();
	}

	// Did we get an ack for the last packet?
	if (uip_acked()) {
		// Record the bytes that were successfully sent
		app->ackedCount += app->sentCount;
		app->sentCount = 0;

		// Check if we're done or need to send more content for this
		// request
		if (app->ackedCount != (int)app->cursor) {
			// Generate the post again to send the next packet of data
			sendRequest();
		}
	}

	if (uip_rexmit()) {
		sendRequest();
	}

 	if (uip_newdata())  {
 		setRXPin(HIGH);

		if (verbose) {
			Serial.print("RX ");
			Serial.print(uip_datalen());
			Serial.print(" bytes from ");
			Serial.println(req->hostName);
		}

		// Check if the sketch cares about the returned data
	 	if ((req->returnFunc) && (uip_datalen() > 0)){
			// Call the sketch's callback function
	 		req->returnFunc((char*)uip_appdata, uip_datalen());
	 	}
 	}

	if (uip_aborted() || uip_timedout() || uip_closed()) {
		if (req != NULL) {
			if (verbose) {
				Serial.print("Ended connection with ");
				Serial.println(req->hostName);
			}

			if (req->returnFunc) {
				// Call the sketch's callback function with 0 bytes to indicate End Of Data
				req->returnFunc((char*)uip_appdata, 0);
			}
			// Remove the request from the connection
			app->request = NULL;
			// Request is no longer active
			req->active = false;
		}
	}
}


char getChar(int nibble) {
	return pgm_read_byte(base64Chars + nibble);
}

void storeBlock(char* src, char* dest, int len) {
	
	dest[0] = getChar(src[0] >> 2);
	dest[1] = getChar(((src[0] & 0x03) << 4) | ((src[1] & 0xf0) >> 4));
	dest[2] = len > 1 ? getChar(((src[1] & 0x0f) << 2) | ((src[2] & 0xc0) >> 6)) : '=';
	dest[3] = len > 2 ? getChar(src[2] & 0x3f ) : '=';
}

char* Server::base64encode(char* data) {
	
	int len = strlen(data);
	int outLenPadded = ((len + 2) / 3) << 2;
	char* out = (char*)malloc(outLenPadded + 1);
	
	char* outP = out;
	while (len > 0) {
		
		storeBlock(data, outP, min(len,3));
		outP += 4;
		data += 3;
		len -= 3;
	}
	*(out + outLenPadded) = 0;
	return out;
}



#endif // ENABLE_CLIENT_MODE



/********* High-level WiServer functions ***********/

/*
 * This function is called by uip whenever a stack event occurs
 */
void server_app_task() {

	// Clear the activity pins
	setTXPin(LOW);
	setRXPin(LOW);

	// Check for an active connection
	if (uip_conn) {
		// Is the connection for local port 80?
		if (uip_conn->lport == HTONS(80)) {
			// Server mode
			server_task_impl();
		} else {
#ifdef ENABLE_CLIENT_MODE
			// Client mode
			client_task_impl();
#endif // ENABLE_CLIENT_MODE
		}
	}
}


/*
 * Called by the sketch's main loop
 */
void Server::server_task() {

	// Run the stack state machine
	stack_process();

	// Run the driver
	zg_drv_process();

#ifdef ENABLE_CLIENT_MODE
	// Check if there is a pending client request
	if (queue) {
		// Attempt to connect to the server
		struct uip_conn *conn = uip_connect(&(queue->ipAddr), queue->port);

		if (conn != NULL) {
#ifdef DEBUG
			Serial.print("Got connection for ");
			Serial.println(queue->hostName);
#endif // DEBUG

			// Attach the request object to its connection
			conn->appstate.request = queue;
			// Move the head of the queue to the next request in the queue
			queue = queue->next;
			// Clear the next pointer of the connected request
			((GETrequest*)conn->appstate.request)->next = NULL;
		}
	}
#endif // ENABLE_CLIENT_MODE
}

// Single instance of the server
Server WiServer;

#endif	/* APP_WISERVER */
