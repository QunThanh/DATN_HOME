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
#define MQTT_SERVER             "192.168.1.88"  //your IP
#define MQTT_PORT               1883
#define MQTT_ID                 "ESP"           // can change
#define TOPPIC_SUB              "S-ESP"         // can change 'ESP', can't change 'S-'
#define TOPPIC_PUB              "P-ESP"         // can change 'ESP', can't change 'P-'


/*  addresses I2C
 *  A2 - A1 - A0
 *  0  - 0  - 0 = 0x20
 *  0  - 0  - 1 = 0x21
 *  0  - 1  - 0 = 0x22
 *  0  - 1  - 1 = 0x23
 *  1  - 0  - 0 = 0x24
 *  1  - 0  - 1 = 0x25
 *  1  - 1  - 0 = 0x26
 *  1  - 1  - 1 = 0x27   // #default LCD
 */
//LCD
#define LCD_I2C_ADDR            0x27           // default address
#define LCD_I2C_NUM_COL         20  
#define LCD_I2C_NUM_ROW         2 
#define LCD_CHANGE_SCREEN_TIME  5              // show screen/5s

// DHT11
#define DHT_PIN                 19              //IO19
#define DHT_GET_DATA_TIME       10              //30s

//Application
#define LED_PIN                 18              //IO18
#define FAN_PIN                 5               //IO5
#define PUMP_PIN                4               //IO4

#define GAS_PIN                 3               //IO3
#define MOI_1_PIN               1               //IO1
#define MOI_2_PIN               2               //IO2
#define GET_SENSOR_DATA_TIME    5               //5s

#endif