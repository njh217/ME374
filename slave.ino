//***************************************************
//  Adafruit MQTT Library ESP8266 Example
//
//  Must use ESP8266 Arduino from:
//    https://github.com/esp8266/Arduino
//
//  Works great with Adafruit's Huzzah ESP board & Feather
//  ----> https://www.adafruit.com/product/2471
//  ----> https://www.adafruit.com/products/2821
//
//  Adafruit invests time and resources providing this open source code,
//  please support Adafruit and open-source hardware by purchasing
//  products from Adafruit!
//
//  Written by Tony DiCola for Adafruit Industries.
//  MIT license, all text above must be included in any redistribution
// ****************************************************/
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "MechatronicsLab"
#define WLAN_PASS       "asapacker1865"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "n_hirdt"
#define AIO_KEY         "6786c74d2791495db9e14f3c2406b6e4"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");
//Adafruit_MQTT_Publish settemp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/settemp");
Adafruit_MQTT_Publish settemp2 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/settemp2");
//Adafruit_MQTT_Publish settemp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/settemp");
Adafruit_MQTT_Publish fspeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/fspeed");

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe settemp1 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/settemp1");

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

int mTemp = 0;
int oTemp = 0;


void setup() {
  Serial.begin(115200);
  delay(10);

  //  Serial.println(F("Adafruit MQTT demo"));

  // Connect to WiFi access point.
  //  Serial.println(); Serial.println();
  //  Serial.print("Connecting to ");
  //  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //    Serial.print(".");
  }
  //  Serial.println();

  //  Serial.println("WiFi connected");
  //  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&settemp1);
}

uint32_t x = 0;

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

  
    Adafruit_MQTT_Subscribe *subscription;
    if ((subscription = mqtt.readSubscription(100))) {
      if (subscription == &settemp1) {
        //Serial.print(F("Got: "));
        mTemp = atoi((char *)settemp1.lastread);
      }
    }

    // Now we can publish stuff!
   // Serial.print(F("\nSending photocell val "));
  //  Serial.print(x);
   // Serial.print("...");
   // if (! photocell.publish(x++)) {
     // Serial.println(F("Failed"));
   // } else {
    //  Serial.println(F("OK!"));
    //}
 

  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');

     int comma_index1 = input.indexOf(',');
     int comma_index2 = input.indexOf(',', comma_index1 + 1);
     int tempNum = (input.substring(0, comma_index1)).toInt(); 
     int stTwo = (input.substring(comma_index1 + 1, comma_index2)).toInt();
     int fan = (input.substring(comma_index2 + 1)).toInt();

    
    temperature.publish(tempNum);
    fspeed.publish(fan);
    settemp2.publish(stTwo);
  
  if (mTemp != oTemp){
    Serial.println(mTemp);
    oTemp = mTemp;
  }
  
  }

  
  

  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
    if(! mqtt.ping()) {
    mqtt.disconnect();
    }
  */
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    //       Serial.println(mqtt.connectErrorString(ret));
    //       Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
  //  Serial.println("MQTT Connected!");
}
