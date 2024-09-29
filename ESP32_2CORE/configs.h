#ifndef __CONFIGS_H__
#define __CONFIGS_H__

//Wifi
#define NUM_WIFI                2               // NUM_WIFI == wifi_pw.length() == wifi_ssid.length()
const char *wifi_ssid[] = {"Thuc Coffee", "Thanh"};      // your wifi name
const char *wifi_pw[] = {"18006230", "aaaaaaaa"}; //your wifi password 

//interrupt
#define PRESS_DEBOUNCE_TIME     500             //500ms

//MQTT
#define MQTT_RECONNECT_TIME     30              //30s
#define MQTT_SEND_DATA_TIME     30              //30s
#define MQTT_SERVER             "192.168.1.88"   //your IP
#define MQTT_PORT               1883
#define MQTT_ID                 "ESP"           // can change
#define TOPPIC_SUB              "S-ESP"         // can change 'ESP', can't change 'S-'
#define TOPPIC_PUB              "P-ESP"         // can change 'ESP', can't change 'P-'

//Application
#define LED_PIN                 18              //IO18
#define FAN_PIN                 5               //IO5
#define PUMP_PIN                5               //IO5

#define DHT_PIN                 19              //IO19

#endif