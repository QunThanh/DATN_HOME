#include <Servo.h>
#include <ESP8266WiFi.h>

//============
const char* ssid = "NTGD";
const char* password = "112233445566";
//============


//########################
void setupWiFi() {
  Serial.printf("Connecting to %s \n", ssid);
  WiFi.begin(ssid, password);  
}





void setup() {
  // put your setup code here, to run once:
  Serial.printf("Connecting to %s \n", ssid);
  WiFi.begin(ssid, password);
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:

}
