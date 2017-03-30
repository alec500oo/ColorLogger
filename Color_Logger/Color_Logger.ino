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
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"


WiFiServer server(23);
Adafruit_BicolorMatrix matrix = Adafruit_BicolorMatrix();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(16, INPUT);
  pinMode(12, OUTPUT); // Blue LED
  pinMode(14, OUTPUT); // Green LED

  digitalWrite(12, LOW);
  digitalWrite(14, HIGH);

  matrix.begin(0x70);
  matrix.setTextWrap(false);
  matrix.setTextSize(1);
  matrix.setTextColor(LED_GREEN);
  matrix.clear();
  matrix.writeDisplay();
  
  server.begin();
}

bool networkActive = false;
long activeTime = 0;

String matrixText = "Stealhead 8176";
void loop() {
  if (networkActive) {
    Serial.println("Creating Network");
    WiFi.softAP("LEDCONTROLLER");
    Serial.print("Host At: ");
    Serial.println(WiFi.softAPIP());

    while (networkActive) {
      yield(); // this needs to be here so it does not crash!!!!
      if (WiFi.softAPgetStationNum() < 1 && (millis() - activeTime) > 30000) {
        Serial.print("No Acctivity... Turning off network... ");
        Serial.println(WiFi.softAPdisconnect(true) ? "Sucess" : "Fail");
        networkActive = false;
        digitalWrite(12, LOW);
        digitalWrite(14, HIGH);
      } else {
        WiFiClient client = server.available();

        if (client) {
          Serial.println("New Client Connected");

          while (client.connected()) {
            yield();

            if (client.available()) {
              String req = client.readStringUntil('\r');
              Serial.println(req);
              client.flush();

              if (req.indexOf("/ChangeString/") != -1) {
                matrixText = req.substring(14);

                Serial.print("Requested String... ");
                Serial.println(matrixText);
              } else if (req.indexOf("/Close/") != -1) {
                client.stop();
              } else {
                Serial.println("Invalid Request");
              }

              client.flush();
              client.print("The result has been logged");
              delay(1);
            }
          }
        }
      }
    }
  }

  if (digitalRead(16) == HIGH) {
    networkActive = true;
    activeTime = millis();
    digitalWrite(12, HIGH);
    digitalWrite(14, LOW);
  }

  int stringLength = (matrixText.length()*7);
  for (int8_t x = 7; x >= -stringLength; x--) {
    matrix.clear();
    matrix.setCursor(x, 0);
    matrix.print(matrixText);
    matrix.writeDisplay();
    delay(100);
  }
}
