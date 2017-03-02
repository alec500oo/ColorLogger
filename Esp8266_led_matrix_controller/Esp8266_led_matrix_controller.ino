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


WiFiServer server(80);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(16, INPUT);
  pinMode(12, OUTPUT); // Blue LED
  pinMode(14, OUTPUT); // Green LED

  digitalWrite(12, LOW);
  digitalWrite(14, HIGH);

  Serial.println(WiFi.softAPIP());
}

bool wifiEnabled = false;
bool networkActive = false;
long activeTime = 0;
void loop() {
  if (networkActive) {
    Serial.println("Creating Network");
    WiFi.softAP("LEDCONTROLLER");
    Serial.print("Host At: ");
    Serial.println(WiFi.softAPIP());

    while (networkActive) {
      yield(); // this needs to be here so it does not crash!!!!
      if (WiFi.softAPgetStationNum() < 1 && (millis() - activeTime) > 30000) {
        Serial.println("No Acctivity... Turning off network");
        Serial.println(WiFi.softAPdisconnect(true) ? "Sucess" : "Fail");
        networkActive = false;
        digitalWrite(12, LOW);
        digitalWrite(14, HIGH);
      } else {
        Serial.print("Number of devices Connected: ");
        Serial.println(WiFi.softAPgetStationNum());
        delay(2000);
      }
    }
  }

  if (digitalRead(16) == HIGH) {
    networkActive = true;
    activeTime = millis();
    digitalWrite(12, HIGH);
    digitalWrite(14, LOW);
  }
}
