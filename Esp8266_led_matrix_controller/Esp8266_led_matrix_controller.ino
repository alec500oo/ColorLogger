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


WiFiServer server(23);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(16, INPUT);
  pinMode(12, OUTPUT); // Blue LED
  pinMode(14, OUTPUT); // Green LED

  digitalWrite(12, LOW);
  digitalWrite(14, HIGH);

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
        Serial.println("New Client Connected");
        
        while(!client) {
          yield();
          delay(1);
        }

        String req = client.readStringUntil('\r');
        Serial.println(req);
        client.flush();

        if(req.indexOf("ChangeString/") != -1) {
          matrixTest = req.substring(14);

          Serial.print("Requested String... ");
          Serial.println(matrixTest);
        }else{
          Serial.println("Invalid Request");
        }

        client.flush();

        client.print("The result has been loged");
        
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
