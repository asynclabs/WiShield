/*
 * LED Control example
 *
 * A simple LED control app using the web server example
 * for the WiShield 1.0
 */

#include <WiShield.h>

#define WIRELESS_MODE_INFRA	1
#define WIRELESS_MODE_ADHOC	2

// Wireless configuration parameters ----------------------------------------
unsigned long local_ip = IP_ADDR(192,168,1,2);
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


// Webpages served by the webserver
//
// Due to the RAM limitation, the largest data packet that the application can
// transmit is limited to 446 bytes. Larger webpages can be broken down into
// smaller chunks and transmitted as shown in the code below to overcome this
// limitation
const prog_char webpage[] PROGMEM = {"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<center><p><h1>LED control</h1></p><p><a href=\"0\">LED 0</a><br><a href=\"1\">LED 1</a></p></center> "};
const prog_char webpage1[] PROGMEM = {"<center><p><h1>This is line 2</h1></p></center>"};
unsigned int webpage_len;



void setup()
{
	// Initialize the WiFi device
	// Resets the WiFi device and sets up the interrupt
	WiFi.init();
	
	// Initialize the Analog In pins 0, 1, 2, 3
	// on the Arduino Duemilanove as outputs to
	// control LEDs
	LED_Init();
	
	Serial.begin(9600);
}

#define APP_IDLE		0
#define APP_MORE_DATA	1

void loop()
{
	char* data;				// variable pointing to the data received from the client
	unsigned int data_len;	//length of data received from the client 
	unsigned char app_state = APP_IDLE;
	
	webpage_len = strlen_P(webpage);	// length of the webpage
	
	WiFi.server_listen(80);		// start server on port 80 (HTTP)

	while(1) {
		WiFi.driver_task();		// run WiFi driver task
		WiFi.stack_task();		// run stack task
		
		data = (char*)WiFi.data_available(&data_len);
			
		if (data_len) {
			// process data
			Serial.println(data);
           
			if (memcmp(data, "GET /", 5) == 0) {
				switch(data[5]) {
                   	case '0':
                   		// toggle LED 0
                   		// Analog In port 0 on the Duemilanove
                   		LED0_toggle();
                   		WiFi.send_data(webpage_len);
                   		break;
                   	case '1':
                   		// toggle LED 1
                   		// Analog In port 1 on the Duemilanove
                   		LED1_toggle();
                   		WiFi.send_data(webpage_len);
                   		break;
                   	case ' ':
                   		// browser requested index.html
                   		// just send main page
                   		WiFi.send_data(webpage_len);
                   		break;
                   	default:
                   		// unknown request, do not respond
						WiFi.send_data(0);
                   		break;
				}
				
				// copy data into TX buffer
				memcpy_P(data, webpage, webpage_len);
				
				// indicate this was the last chunk for TX
				WiFi.set_more_data(1);
			}
           
			// we run the stack task again here to allow the stack
			// to package the app data and set it up for TX when
			// the driver task runs the next time
			WiFi.stack_task();
		}

#if 0
		// This code block implements multiple chunk TX
				
		switch (app_state) {
		case APP_IDLE:
			data = (char*)WiFi.data_available(&data_len);
			
			if (data_len) {
				// process data
				Serial.println(data);
				
				// copy data into TX buffer
				memcpy_P(data, webpage, 104);
				WiFi.send_data(104);
				
				// indicate that the webpage has more data to TX
				WiFi.set_more_data(0);
				
				app_state = APP_MORE_DATA;
				
				// we run the stack task again here to allow the stack
				// to package the app data and set it up for TX when
				// the driver task runs the next time
				WiFi.stack_task();
			}
			break;
		case APP_MORE_DATA:
			data = (char*)WiFi.data_available(&data_len);
			
			// copy more webpage data into TX buffer
			memcpy_P(data, webpage1, 47);
			WiFi.send_data(47);
			
			// indicate end of webpage data
			WiFi.set_more_data(1);
			
			app_state = APP_IDLE;
			
			// we run the stack task again here to allow the stack
			// to package the app data and set it up for TX when
			// the driver task runs the next time
			WiFi.stack_task();
			break;
		}
#endif
	}
}
