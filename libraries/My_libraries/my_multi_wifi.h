#ifndef __MY_MULTI_WIFI_H__
#define __MY_MULTI_WIFI_H__

// ######### Include Librarys #########
#include <WiFi.h>
#include <WiFiMulti.h>
// ####################################

// ######### Define Librarys ##########
WiFiMulti wifiMulti;
// ####################################

// ######### Define Variables #########
#ifndef PIN_STATUS_LED
#define PIN_STATUS_LED          2
#endif

#ifndef NUM_WIFI
#define NUM_WIFI                2
#endif

#ifndef LED_STT_DELAY_TIME
#define LED_STT_DELAY_TIME      60
#endif


unsigned long current_time_in_wifi = 0;
unsigned long old_time_in_wifi = 0;

// ####################################


// ######### setup Status Led  #############
void setupStatusLed()
{
    pinMode(PIN_STATUS_LED,OUTPUT);
    digitalWrite(PIN_STATUS_LED, LOW);
}
// #######################################


// ######### setup Wifi #############
void showInforWifi()
{
    Serial.println("_________WIFI_________");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("RSSI: ");
    Serial.println(WiFi.RSSI());
    Serial.println("______________________");
}

void connectWiFiMulti()
{
    digitalWrite(PIN_STATUS_LED, HIGH);
    Serial.print("Connecting Wifi...");
    while (wifiMulti.run() != WL_CONNECTED)
    {
        digitalWrite(PIN_STATUS_LED, LOW);
        Serial.print(".");
        if (millis() > 15000)
            break;
    }
    Serial.println("");

    showInforWifi();

    // if (wifiMulti.run() == WL_CONNECTED)
    // {
    //     showInforWifi();
    // }
    // else
    // {
    //     Serial.println("WiFi not connected!");
    //     digitalWrite(PIN_STATUS_LED, LOW);
    // }
}

void setupWiFiMulti()
{
    // setup status led (ESP32 GPIO2)
    setupStatusLed();

    // setup mode Wifi
    WiFi.mode(WIFI_STA);

    // Disable WiFi
    if (NUM_WIFI <= 0)
        return;
    
    // add list wifi
    for (byte i = 0; i < NUM_WIFI; i++)
        wifiMulti.addAP(wifi_ssid[i], wifi_pw[i]);

    connectWiFiMulti();
}

void loopWiFiMulti()
{
    if (NUM_WIFI <= 0)
        return;

    // check connect. 1m/1time
    current_time_in_wifi = millis();
    if (current_time_in_wifi - old_time_in_wifi < LED_STT_DELAY_TIME * 1000)
        return;
    
    old_time_in_wifi = millis();
    if (wifiMulti.run() == WL_CONNECTED)
    { 
        int stt = digitalRead(PIN_STATUS_LED);
        digitalWrite(PIN_STATUS_LED, !stt); // on ledstatus
        return;
    }
    
    // if Wifi disconnet, try to connect other
    Serial.println("Not connected to WiFi. Re-connecting now...");
    connectWiFiMulti();
}
// ####################################

#endif