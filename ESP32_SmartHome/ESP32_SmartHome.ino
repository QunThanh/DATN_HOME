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
  if(command.length() < 0)
    return;    
  
  int cmd_index = -2;
  cmd_index = command.indexOf(";");
  
  //handel without ";"
  //eg: on
  if(cmd_index < 0)
  {
    if(command == "on")
      onAllLed();
    if(command == "off")
      offAllLed(); 
   
    is_report_now = true;
    return;
  }

  //handel with ";"
  //eg: 18;on (io18 on)
  int io = command.substring(0,cmd_index).toInt();
  String sub_cmd = command.substring(cmd_index + 1);
  
  if(sub_cmd == "on")
    onled(io);

  if(sub_cmd == "off")
    offLed(io);

  if(sub_cmd == "toggle")
    toggleLed(io);  

  is_report_now = true;
  return;
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