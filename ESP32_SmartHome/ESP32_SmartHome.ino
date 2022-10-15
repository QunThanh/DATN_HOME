#include "configs.h"

//##### include your libraries inthere ########
#include "my_multi_wifi.h"
#include "my_ota.h"
#include "my_interrupt.h"
#include "my_mqtt.h"
//#############################################

#include "app.h"

// #include "setup_esp32.h"


// handle Command form Server
void executeMqttCommand(String command) 
{
  Serial.println("command from Server: " + command);
  
}

void setup()
{
  Serial.begin(112500);
  pinMode(2, OUTPUT); //this's LED of board ESP. use it for notification. So don't change
  
  setupApp();
  delay(50);
  
  setupWiFiMulti();
  delay(50);

  setupOTA();
  delay(50);

  setupInterrupt();
  delay(50);

  setupMQTT();
  delay(50);
}

void loop()
{
  loopApp();
  delay(30);

  loopMQTT();
  delay(30);

  loopOTA();
  delay(30);

  loopWiFiMulti();
  delay(30);
}