#ifndef __CONFIGS_H__
#define __CONFIGS_H__

//Wifi
const char *wifi_ssid[]{"THENAM", "NTGD"};      // your wifi name
const char *wifi_pw[] = {"0964941600", "112233445566"}; //your wifi password 
#define NUM_WIFI                2               // NUM_WIFI == wifi_pw.length() == wifi_ssid.length()
#define LED_STT_DELAY_TIME      60              // 60s

//ota
#define OTA_HOSTNAME            "ESP-%06X"      // can change 'ESP', can't change '-%06X'

//interrupt
int list_io_interrupt[] = {21};                  // list_io_interrupt.length() == NUM_PIN_INTERRUPT
#define NUM_PIN_INTERRUPT       1                   
#define EN_INTERRUPT            true            // if use interrupt, will be change false to true
#define MODE_INTERRUPT          RISING          // 5 mode: RISING, FALING, LOW, HIGH, CHANGE

//MQTT
#define DELAY_TIME_MQTT         1*60            //1m
#define MQTT_SERVER             "192.168.1.3"   //your IP
#define MQTT_PORT               1883
#define MQTT_ID                 "ESP"           // can change
#define TOPPIC_SUB              "S-ESP"         // can change 'ESP', can't change 'S-'
#define TOPPIC_PUB              "P-ESP"         // can change 'ESP', can't change 'P-'

//Application
#define RED_LED_PIN             18              //IO18
#define GREEN_LED_PIN           5               //IO5

#define DHT_PIN                 19              //IO19
#define DHTTYPE                 DHT11           //type DHT

#endif