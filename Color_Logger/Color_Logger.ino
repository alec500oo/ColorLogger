#include <SD.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

#include <Wire.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_GFX.h>
#include <Adafruit_TCS34725.h>

//Define the I/O pins
#define SDCS 16 // Chip select for the sd card
#define BUTTON 0 // Tac switch, same as Huzzah switch
#define GREENLED 15
#define BLUELED 2

enum state {
  NORMAL,
  NETWORKACTIVE,
  CLIENTCONNECTED
};

WiFiServer server(23);
WiFiClient client;
Adafruit_BicolorMatrix matrix = Adafruit_BicolorMatrix();
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

void setup() {
  // put your setup code here, to run once:
  //Setup serial and the led
  Serial.begin(115200);
  pinMode(BUTTON, INPUT);
  pinMode(BLUELED, OUTPUT);
  pinMode(GREENLED, OUTPUT);

  digitalWrite(BLUELED, LOW);
  digitalWrite(GREENLED, HIGH);

  //start the server for changing settings
  server.begin();
}

enum state networkState = NORMAL;
long activeTime = 0;

void loop() {
  switch (networkState) {
    case NETWORKACTIVE:
      //Check for network actifity, if none stop the network
      if (WiFi.softAPgetStationNum() < 1 && (millis() - activeTime) > 30000) {
        Serial.print("No Acctivity... Turning off network... ");
        Serial.println(WiFi.softAPdisconnect(true) ? "Sucess" : "Failiure");
        digitalWrite(BLUELED, LOW);
        digitalWrite(GREENLED, HIGH);
        networkState = NORMAL;
      } else {
        //get a new client
        client = server.available();
        if (client) {
          Serial.print("New Client Connected... ");
          Serial.println("Waiting for a command");
          client.println("Waiting for a command");
          networkState = CLIENTCONNECTED;
        }
      }
      break;
    case CLIENTCONNECTED:
      if (client.connected()) {
        if (client.available()) {
          String req = client.readStringUntil('\r');
          Serial.println(req);
          client.flush();

          if (req.indexOf("/ChangeString/") != -1) {

          } else if (req.indexOf("/Close/") != -1) {
            client.println("CLOSING CONNECTION");
            delay(100);
            client.stop();
            Serial.print("WiFi AP turned off... ");
            Serial.println(WiFi.softAPdisconnect(true) ? "Success" : "Failiure");
            digitalWrite(BLUELED, LOW);
            digitalWrite(GREENLED, HIGH);
            networkState = NORMAL;
          } else if (req.indexOf("/EnableColor/") != -1) {
            if (tcs.begin()) {
              Serial.println("Color Sensor Active");
              client.println("Color Sensor Active");
            } else {
              Serial.println("Color Sensor Activation Failed");
              client.println("Color Sensor Activation Failed");
            }
          } else if (req.indexOf("/LedOn/") != -1) {
            //Turn on the color sensor led
            tcs.setInterrupt(false);
          } else if (req.indexOf("/LedOff/") != -1) {
            //Turn off the color sensor led
            tcs.setInterrupt(true);
          } else {
            client.println("Invalid Request");
            Serial.println("Invalid Request");
          }
        }
      } else {
        networkState = NORMAL;
      }
      break;
    case NORMAL:
      if (digitalRead(BUTTON) == LOW) {
        activeTime = millis();
        //switch the led state
        digitalWrite(BLUELED, HIGH);
        digitalWrite(GREENLED, LOW);
        //create the local wifi network
        Serial.println("Creating Network");
        WiFi.softAP("LEDCONTROLLER");
        Serial.print("Host At: ");
        Serial.println(WiFi.softAPIP());
        networkState = NETWORKACTIVE;
      }
      break;
  }
}
