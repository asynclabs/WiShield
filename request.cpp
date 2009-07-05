/******************************************************************************
 
 Filename:		request.cpp
 Description:	Request code for the WiServer library
 
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
 Mark A. Patel		06/20/2009	Initial version
 
 
 *****************************************************************************/

#include "WiServer.h"

#ifdef ENABLE_CLIENT_MODE

// IP, host name and URL for the TWEET class
uint8 twitterIP[] = {128, 121, 146, 100};
char twitterHost[] = {"twitter.com"};
char twitterURL[] = {"/statuses/update.xml"};


GETrequest::GETrequest(uint8* ipAddr, int port, char* hostName, char* URL) {
	// Store IP address using the uIP type
	uip_ipaddr(this->ipAddr, ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
	// Store port in network order
	this->port = htons(port);
	// Store host name and URL
	this->hostName = hostName;
	this->URL = URL;
	
	// Set remaining members to NULL
	this->auth = NULL;
	this->returnFunc = NULL;
	this->active = false;
	this->body = NULL;
	this->bodyPreamble = NULL;
	this->next = NULL;
}

void GETrequest::setReturnFunc(returnFunction func) {
	if (!this->active) this->returnFunc = func;
}

void GETrequest::setAuth(char* auth) {
	if (!this->active) this->auth = auth;
}

void GETrequest::setURL(char* URL) {
	if (!this->active) this->URL = URL;
}

void GETrequest::submit() {	
	// Ignore submit request if already active
	if (!this->active) WiServer.submitRequest(this);
}

boolean GETrequest::isActive() {	
	return this->active;
}




POSTrequest::POSTrequest(uint8* ipAddr, int port, char* hostName, char* URL, bodyFunction body) : 
    // Create a request with the IP, port, host name and URL
	GETrequest (ipAddr, port, hostName, URL) {
		// Set the body callback function
		this->body = body;
}
		
void POSTrequest::setBodyFunc(bodyFunction body) {
	if (!this->active) this->body = body;
}

TWEETrequest::TWEETrequest(char* auth, bodyFunction message) : 
    // Create a POST request with the Twitter-specific data
	POSTrequest (twitterIP, 80, twitterHost, twitterURL, message) {
		// Set the auth string
		this->auth = auth;
		// Automatically insert 'status=' before calling the body function
		this->bodyPreamble = (char*)"status=";
}

#endif // ENABLE_CLIENT_MODE
