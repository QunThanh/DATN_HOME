#include "configs.h"
#include "setup_esp32.h"
#include "app.h"

// handle Command form Server
void executeMqttCommand(String command) 
{
  Serial.println("command from Server: " + command);
}

