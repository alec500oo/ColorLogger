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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(16, INPUT);
  pinMode(12, OUTPUT); // Blue LED
  pinMode(14, OUTPUT); // Green LED

  digitalWrite(14, HIGH);
}

bool wifiEnabled = false;
bool buttonPressed = false;
long waitTime = 0;
void loop() {
  // put your main code here, to run repeatedly:
  if(digitalRead(16)) {
    buttonPressed = true;
    waitTime = millis();
  } else {
    buttonPressed = false;
  }
  while(buttonPressed) {
    if((millis() - waitTime) >= 10000) {
      Serial.println("WiFi Activating");
      digitalWrite(14, LOW);
      digitalWrite(12, HIGH);

      Serial.println("Setting Up Soft AP ....");
      Serial.println(WiFi.softAP("LEDBADGE") ? "Ready" : "Failed"); 
    }

    Serial.printf("Soft Ap number of connections = %d\n", WiFi.softAPgetStationNum());
    delay(3000);
  }

}
