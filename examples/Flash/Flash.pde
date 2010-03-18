/*
 * Web Server using Flash
 *
 * A simple web server example using the WiShield 2.0
 * that reads the manufacturing ID from the Flash and
 * displays it on a webpage.
 */

#include <WiShield.h>
#include <dataflash.h>

#define WIRELESS_MODE_INFRA    1
#define WIRELESS_MODE_ADHOC    2

#define FLASH_SLAVE_SELECT 7
#define WIFI_SLAVE_SELECT  10

// Wireless configuration parameters ----------------------------------------
unsigned char local_ip[] = {192,168,1,2};	// IP address of WiShield
unsigned char gateway_ip[] = {192,168,1,1};	// router or gateway IP address
unsigned char subnet_mask[] = {255,255,255,0};	// subnet mask for the local network
const prog_char ssid[] PROGMEM = {"ASYNCLABS"};	// max 32 bytes

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

unsigned char mfg_id[4];

void setup()
{
  initShield();  
}

// This is the webpage that is served up by the webserver
const prog_char webpage[] PROGMEM = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
									"<html><body style=\"margin:100px\">"
									"<center>"
									"<h1>Flash Mfg ID: 000 000 000</h1>"
									"<form method=post action=\"/upload\" enctype=\"multipart/form-data\">"
									"<b>FS Image Upload</b>"
									"<p>"
									"<input type=file name=i size=40> &nbsp; <input type=submit value=\"Upload\">"
									"</form>"
									"</center>"
									"</body></html>";

void loop()
{ 
  dflash.read_id(mfg_id);

  WiFi.run();
}

void initShield()
{
  // there is some contention on the SPI between the flash and wifi chip,
  // so disable both devices at the beginning until they are properly
  // initialized by their respective libraries
  pinMode(FLASH_SLAVE_SELECT, OUTPUT);
  digitalWrite(FLASH_SLAVE_SELECT, HIGH);
  pinMode(WIFI_SLAVE_SELECT, OUTPUT);
  digitalWrite(WIFI_SLAVE_SELECT, HIGH);

  // now init dataflash
  dflash.init(FLASH_SLAVE_SELECT);

  // now init wifi
  WiFi.init();
}

