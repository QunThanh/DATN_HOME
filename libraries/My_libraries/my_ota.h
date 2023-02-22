#ifndef __MY_OTA_H__
#define __MY_OTA_H__

// ######### Include Librarys #########
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
// ####################################

// ######### Define Variables #########
#ifndef OTA_HOSTNAME
    #define OTA_HOSTNAME "ESP-%06X"
#endif

// bool enable_ota = false;
// ####################################

// ######### Define function #########
// get mac addr
uint64_t getChipId()
{
    uint64_t chipid = ESP.getEfuseMac();      
    uint32_t discard_two_bytes = (uint32_t)(chipid >> 16);
    return discard_two_bytes;
}

void showInforOTA(){

    // Hostname defaults is: esp3232-[MAC]
    
    // Port defaults to 3232
    // ArduinoOTA.setPort(3232);

    char hostname[24];
    ArduinoOTA.setHostname(hostname);
    sprintf(hostname, OTA_HOSTNAME, getChipId());

    Serial.println("______OTA______");
    Serial.print("[OTA] Hostname: ");
    Serial.println(hostname);
    Serial.println("_______________");
}
// ####################################

// ######### setup Basic OTA #########
void setupOTA(){

    if(wifiMulti.run() != WL_CONNECTED)
        return;

    // enable_ota = true;
    ArduinoOTA
        .onStart([](){
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH)
                type = "sketch";
            else // U_SPIFFS
                type = "filesystem";
            // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
            Serial.println("Start updating " + type); 
        })
        
        .onEnd([](){
            Serial.println("\nEnd"); 
        })
        
        .onProgress([](unsigned int progress, unsigned int total){ 
            Serial.printf("Progress: %u%%\r", (progress / (total / 100))); 
        })
        
        .onError([](ota_error_t error){
            Serial.printf("Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
            else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
            else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
            else if (error == OTA_END_ERROR) Serial.println("End Failed"); 
        });

    //show information OTA and init OTA 
    showInforOTA();
    ArduinoOTA.begin();
}

void loopOTA(){
    // if(enable_ota)      
    ArduinoOTA.handle();
}
// ####################################

#endif

