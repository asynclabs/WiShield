/*
 * A simple sketch that uses WiServer to send a tweet with the current system time every 5 minutes
 */

#include <WiServer.h>

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
prog_uchar wep_keys[] PROGMEM = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,	// Key 0
				  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Key 1
				  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Key 2
				  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	// Key 3
				};

// setup the wireless mode
// infrastructure - connect to AP
// adhoc - connect to another WiFi device
unsigned char wireless_mode = WIRELESS_MODE_INFRA;

unsigned char ssid_len;
unsigned char security_passphrase_len;
// End of wireless configuration parameters ----------------------------------------


// Auth string for the Twitter account
char* auth = "[YOUR DATA HERE]"; // Base64 encoded USERNAME:PASSWORD


// This function generates a message with the current system time
void currentTime() {
   WiServer.print("Arduino has been running for ");
   WiServer.printTime(millis());
}

// A request that sends a Tweet using the currentTime function
TWEETrequest sentMyTweet(auth, currentTime);


void setup() {
    // Initialize WiServer (we'll pass NULL for the page serving function since we don't need to serve web pages) 
  WiServer.init(NULL);
  
  // Enable Serial output and ask WiServer to generate log messages (optional)
  Serial.begin(57600);
  WiServer.enableVerboseMode(true);
}


// Time (in millis) when the next tweet should be sent 
long tweetTime = 0;

void loop(){

  // Check if it's time to sent a tweet
  if (millis() >= tweetTime) {
    sentMyTweet.submit();    
    // Send next tweet 5 minutes from now
    tweetTime += 1000 * 60 * 5;
  }
  
  // Run WiServer
  WiServer.server_task();
 
  delay(10);
}
