/*
 * Web Client
 *
 * A simple web client example using the WiShield 1.0
 */

#include <WiShield.h>

#define WIRELESS_MODE_INFRA	1
#define WIRELESS_MODE_ADHOC	2

// Wireless configuration parameters ----------------------------------------
unsigned char local_ip[] = {192,168,1,2};	// IP address of WiShield
unsigned char gateway_ip[] = {192,168,1,1};	// router or gateway IP address
unsigned char subnet_mask[] = {255,255,255,0};	// subnet mask for the local network
const prog_char ssid[] PROGMEM = {"ASYNCLABS"};		// max 32 bytes

unsigned char security_type = 0;	// 0 - open; 1 - WEP; 2 - WPA; 3 - WPA2

// WPA/WPA2 passphrase
const prog_char security_passphrase[] PROGMEM = {"12345678"};	// max 64 characters

// WEP 128-bit keys
// sample HEX keys
prog_uchar wep_keys[] PROGMEM = {	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,	// Key 0
									0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00,	// Key 1
									0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00,	// Key 2
									0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00	// Key 3
								};

// setup the wireless mode
// infrastructure - connect to AP
// adhoc - connect to another WiFi device
unsigned char wireless_mode = WIRELESS_MODE_INFRA;

unsigned char ssid_len;
unsigned char security_passphrase_len;
//---------------------------------------------------------------------------

void setup()
{
	WiFi.init();
}

unsigned char loop_cnt = 0;

// The stack does not have support for DNS and therefore cannot resolve
// host names. It needs actual IP addresses of the servers. This info
// can be obtained by executing, for example, $ ping twitter.com on
// a terminal on your PC
//char google_ip[] = {74,125,67,100};	// Google
char twitter_ip[] = {128,121,146,100};	// Twitter

// This string can be used to send a request to Twitter.com to update your status
// It will need a valid Authorization string which can be derived from your
// Twitter.com username and password using Base64 algorithm
// See, http://en.wikipedia.org/wiki/Basic_access_authentication
// You need to replace <-!!-Authorization String-!!-> with a valid string before
// using this sample sketch.
// The Content-Length variable should equal the length of the data string
// In the example below, "Content-Length: 21" corresponds to "status=Ready to sleep"
const prog_char twitter[] PROGMEM = {"POST /statuses/update.xml HTTP/1.1\r\nAuthorization: Basic <-!!-Authorization String-!!->\r\nUser-Agent: uIP/1.0\r\nHost: twitter.com\r\nContent-Length: 21\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\nstatus=Ready to sleep"};

void loop()
{
	// if this is the first iteration
	// send the request
	if (loop_cnt == 0) {
		webclient_get(twitter_ip, 80, "/");
		loop_cnt = 1;
	}
	
	WiFi.run();
}
