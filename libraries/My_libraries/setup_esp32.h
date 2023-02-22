#ifndef __SETUP_ESP32_H__
#define __SETUP_ESP32_H__

#include "my_multi_wifi.h"
#include "my_ota.h"
#include "my_interrupt.h"
#include "my_mqtt.h"



#define BAUD_RATE           115200

void setupApp();
void loopApp();

void setup()
{
    Serial.begin(BAUD_RATE);
    pinMode(2, OUTPUT);
    
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

#endif