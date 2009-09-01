/******************************************************************************

  Filename:		WiSever.h
  Description:	Main header file for the WiServer library

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

   Mark A. Patel		06/22/2009	Revised client API


 *****************************************************************************/

#ifndef WISERVER_H_
#define WISERVER_H_

extern "C" {
	#include "witypes.h"
	#include "server.h"
	#include "config.h"
	#include "uip.h"
}

#include "Print.h"


/*
 * Function for handling data returned by a POST or GET request
 */
typedef void (*returnFunction)(char* data, int len);

/*
 * Function for serving web pages
 */
typedef boolean (*pageServingFunction)(char* URL);

/*
 * Function for providing the body of a POST request
 */
typedef void (*bodyFunction)();



#ifdef ENABLE_CLIENT_MODE
/*********** Classes for client-mode functionality *********/

/*
 * Class that encapsulates an HTTP GET request
 */
class GETrequest
	{
	public:

		/*
		 * Creates a new GETrequest with the provided IP, port, host name and URL
		 */
		GETrequest(uint8* ipAddr, int port, char* hostName, char* URL);

		/*
		 * Submits the request and prevents further changes to the request until it has been
		 * processed.
		 */
		void submit();

		/*
		 * Sets the function that should be called with data returned by the server.
		 */
		void setReturnFunc(returnFunction func);

		/*
		 * Sets the authorization string for the request.  Calls to this method will be ignored if the request
		 * has been submitted and is still being processed by WiServer.
		 */
		void setAuth(char* auth);

		/*
		 * Checks if this request is currently being processed by the WiServer (i.e. it is awaiting a connection
		 * or is currently connected and communicating with the server).  If it is, any calls that attempt
		 * to change the properties of the request will be ignored.
		 */
		boolean isActive();

		/*
		 * Sets the URL for the request.  Calls to this method will be ignored if the request
		 * has been submitted and is currently being processed by WiServer.
		 */
		void setURL(char* URL);

	    // Server IP address (network byte order)
		uip_ipaddr_t ipAddr;
		// Server port number
		int port;
		// Server domain name
		char* hostName;
		// Request URL
		char* URL;
		// Authorization string (may be NULL)
	    char* auth;
	    // Return value callback function (may be NULL)
		returnFunction returnFunc;
	    // Indicates if the request is currently active (i.e. has a connection)
		boolean active;
	    // Body data callback function (may be NULL)
	    bodyFunction body;
	    // Body preamble (may be NULL)
	    char* bodyPreamble;
		// Pointer to the next request in the queue
	    GETrequest* next;
	};


/*
 * Class that encapsulates aN HTTP POST request
 */
class POSTrequest : public GETrequest
	{
	public:

		/*
		 * Creates a new POSTrequest with the provided IP, port, host name, URL and body function.
		 */
		POSTrequest(uint8* ipAddr, int port, char* hostName, char* URL, bodyFunction body);

		void setBodyFunc(bodyFunction body);

	};


/*
 * Class that encapsulates a TWEET request (basically a POST request populated with Twitter-specific data)
 */
class TWEETrequest : public POSTrequest
	{
	public:

		/*
		 * Creates a new TWEETrequest with the provided auth string and message function.
		 * The body function will be called to provide the contents of the message; the
		 * 'status=' prefix is automatically inserted before the message.
		 */
		TWEETrequest(char* auth, bodyFunction message);
	};

#endif // ENABLE_CLIENT_MODE


class Server: public Print
{
	public:

		/**
		 * Inits the server with the provided page serving function
		 *
		 * @param pageServerFunc name of the sketch's page serving function
		 */
		void init(pageServingFunction function);

	    /*
		 * Enables or disables verbose mode.  If verbose mode is true, then WiServer
		 * will output log info via the Serial class.  Verbose mode is disabled by
		 * default, but is automatically enabled if DEBUG is defined
		 */
		void enableVerboseMode(boolean enable);

		/**
		 * The server task method (must be called in the main loop to run the WiServer)
		 */
		void server_task();

		/**
		 * Writes a single byte to the current connection buffer
		 */
		virtual void write(uint8_t);

		/**
		 * Prints a string that is stored in program memory
		 */
		void print_P(const char[]);

		/**
		 * Prints a string that is stored in program memory followed by a line feed
		 */
		void println_P(const char[]);

		/**
		 * Writes data of a specified length that is stored in program memory
		 */
		void write_P(const char[], int len);

		/**
		 * Prints a time value in the form HH:MM:SS.  The time value is in milliseconds.
		 *
		 */
		void printTime(long t);

		/**
		 * Indicates if a page is currently being sent, and that a subsequent call to the page
		 * serving function may occur to request more data or to retransmit data that was lost
		 * in the network.  Changes to the content of the page should only be made if this method
		 * returns false.
		 */
		boolean sendInProgress();

		/**
		 * Checks if the client for the current server request resides on the same local network
		 * as the server.  This information can be used to control access to data
		 * and capabilities based on where the client is located.  The check is made
		 * by comparing the server and client addresses using the subnet mask.
		 * <br>
		 * For example, a web page front-end for a HVAC system might display read-only data
		 * to all clients, but may restrict changes to the settings to only those clients
		 * connected via the building's local network.
		 * <br>
		 * Note that security checks based on the client IP address are not 100% reliable,
		 * so this feature should not be relied upon to control access to sensitive data!
		 */
		boolean clientIsLocal();

		/**
		 * Sets the pins used to indicate TX and RX activity over the network.
		 * These pins will be set to a HIGH output when activity occurs, and may
		 * be connected to LEDs via a suitable resistor to provide indicator lights.
		 * A value of -1 disables activity indication.
		 */
		void setIndicatorPins(int tx, int rx);
	
#ifdef ENABLE_CLIENT_MODE

		/*
		 * Called by request classes to submit themselves to the queue
		 */
		void submitRequest(GETrequest *req);
	
    	char* base64encode(char* data);

	   	

#endif // ENABLE_CLIENT_MODE

};

// Single instance of the Server
extern Server WiServer;



#endif /* WISERVER_H_ */
