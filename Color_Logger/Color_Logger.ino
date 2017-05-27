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
#define SDCS 16     // Chip select for the sd card
#define BUTTON 0    // Tac switch, same as Huzzah switch
#define GREENLED 15 //Green led pin
#define BLUELED 2   //Blue led pin

//Define matrix address
#define MATRIX_ADDR 0x70

enum state {
  NORMAL,
  NETWORKACTIVE,
  CLIENTCONNECTED,
  COLORACTIVE,
  MATRIXACTIVE
};

WiFiServer server(23);
WiFiClient client;
Adafruit_BicolorMatrix matrix = Adafruit_BicolorMatrix();
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

boolean broadcastActive = false;
boolean matrixActive = false;
boolean clientConnected = false;

String username, pass;
String matrixText = "Hello World";
int x = 7;
void setup() {
  // put your setup code here, to run once:
  //Setup serial, led, and button
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
long broadcastTime = 0;

uint16_t red, green, blue, clear;

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
          clientConnected = true;
        }
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
      if (!matrixActive) {
        break;
      }
    case MATRIXACTIVE:
      {
        int stringLength = (matrixText.length() * 7);
        for (int8_t x = 7; x >= -stringLength; x--) {
          matrix.clear();
          matrix.setCursor(x, 0);
          matrix.print(matrixText);
          matrix.writeDisplay();
          if ((clientConnected && client.available()) || (networkState == NORMAL && digitalRead(BUTTON) == LOW)) {
            break;
          } else {
            delay(100);
          }
        }
      }
      if (!clientConnected) {
        break;
      }
    case CLIENTCONNECTED:
      if (client.connected()) {
        if (client.available()) {
          String req = client.readStringUntil('\r');
          Serial.println(req);
          client.flush();

          if (req.indexOf("/ConnectToNetwork/") != -1) {
            String temp = req.substring(18);
            int passIndex = temp.indexOf("/");
            username = temp.substring(0, passIndex);
            pass = temp.substring(passIndex + 1);
            Serial.println(username);
            Serial.println(pass);
          } else if (req.indexOf("/ChangeString/") != -1) {
            matrixText = req.substring(14);
          } else if (req.indexOf("/Close/") != -1) {
            client.println("CLOSING CONNECTION");
            delay(100);
            client.stop();
            Serial.print("WiFi AP turned off... ");
            Serial.println(WiFi.softAPdisconnect(true) ? "Success" : "Failiure");
            digitalWrite(BLUELED, LOW);
            digitalWrite(GREENLED, HIGH);
            networkState = NORMAL;
            clientConnected = false;
          } else if (req.indexOf("/EnableMatrix/") != -1) { //Enable the led matrix for use and setup scrolling with the deafault string.
            Serial.print("Enabling led matrix with string: ");
            Serial.println(matrixText);
            client.print("Enabling led matrix with string: ");
            client.println(matrixText);

            matrix.begin(MATRIX_ADDR);
            matrix.clear();
            matrix.setTextWrap(false);
            matrix.setTextSize(1);
            matrix.setTextColor(LED_RED);
            matrix.writeDisplay();
            networkState = MATRIXACTIVE;
            matrixActive = true;
          } else if (req.indexOf("/DisableMatrix/") != -1) { //Disable the matrix
            matrix.clear();
            matrix.writeDisplay();
            networkState = CLIENTCONNECTED;
            matrixActive = false;
        } else if (req.indexOf("/EnableColor/") != -1) {
          if (tcs.begin()) {
              Serial.println("Color Sensor Active");
              client.println("Color Sensor Active");
              broadcastActive = true;
              broadcastTime = millis();
            } else {
              Serial.println("Color Sensor Activation Failed");
              client.println("Color Sensor Activation Failed");
            }
          } else if (req.indexOf("/StartBroadcast/") != -1) {
          broadcastActive = true;
          broadcastTime = millis();
          } else if (req.indexOf("/StopBroadcast/") != -1) {
          broadcastActive = false;
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

      //      if (!broadcastActive) {
      //        break;
      //      };
      break;
    case COLORACTIVE:
      if ((millis() - broadcastTime) >= 1000) {
        tcs.getRawData(&red, &green, &blue, &clear);
        String colorData = String('/');
        colorData += red;
        colorData += String(',');
        colorData += green;
        colorData += String(',');
        colorData += blue;
        colorData += String(',');
        colorData += clear;
        colorData += String('/');

        Serial.println(colorData);
        client.println(colorData);
        broadcastTime = millis();
      }
      break;
  }
}
