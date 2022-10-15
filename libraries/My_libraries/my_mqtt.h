#ifndef __MQTT_SETUP_H__
#define __MQTT_SETUP_H__

#include <SPI.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

WiFiClient client;
PubSubClient mqtt_client(client);

#ifndef DELAY_TIME_MQTT
#define DELAY_TIME_MQTT         1*60            //1m
#endif

#ifndef MQTT_SERVER
#define MQTT_SERVER             "localhost"
#endif

#ifndef MQTT_PORT
#define MQTT_PORT               1883
#endif

#ifndef MQTT_ID
#define MQTT_ID                 "ESP"
#endif

#ifndef TOPPIC_SUB
#define TOPPIC_SUB              "S-ESP"
#endif

#ifndef TOPPIC_PUB
#define TOPPIC_PUB              "P-ESP"
#endif


unsigned long old_time_mqtt = 0;
String buffer_data_from_sever = "";
const char *mqttserver = MQTT_SERVER;
const int mqttport = MQTT_PORT;
const char *mqttid = MQTT_ID;
const char *toppicsub = TOPPIC_SUB;
const char *toppicpub = TOPPIC_PUB;

// write function in app.h
void executeMqttCommand(String command);
void statusMQTT();

// call function in app.h if u would send data to server
void mqttSendServer(String data)
{
    if (!mqtt_client.connected())
    {
        statusMQTT();
        return;
    }
        
    if ( millis() - old_time_mqtt > DELAY_TIME_MQTT * 1000 )
    {
        mqtt_client.publish(toppicpub, data.c_str());
        old_time_mqtt = millis();
        return;
    }
}

void mqttSendServerNow(String data)
{
    mqtt_client.publish(toppicpub, data.c_str());
}

void statusMQTT() 
{
    int state = mqtt_client.state();
    switch (state)
    {
        case -4:
            Serial.println("MQTT_CONNECTION_TIMEOUT");
            break;
        case -3:
            Serial.println("MQTT_CONNECTION_LOST");
            break;
        case -2:
            Serial.println("MQTT_CONNECT_FAILED");
            break;
        case -1:
            Serial.println("MQTT_DISCONNECTED");
            break;
        case 0:
            Serial.println("MQTT_CONNECTED");
            break;
        case 1:
            Serial.println("MQTT_CONNECT_BAD_PROTOCOL");
            break;
        case 2:
            Serial.println("MQTT_CONNECT_BAD_CLIENT_ID");
            break;
        case 3:
            Serial.println("MQTT_CONNECT_UNAVAILABLE");
            break;
        case 4:
            Serial.println("MQTT_CONNECT_BAD_CREDENTIALS");
            break;
        case 5:
            Serial.println("MQTT_CONNECT_UNAUTHORIZED");
            break;

        default:
            Serial.println("unknown!!!");
            break;
    }
}

void showInforMQTT(){
    Serial.println("_________MQTT__________");
    Serial.println("Connected MQTT");
    Serial.print("MQTT SERVER:");
    Serial.print(String(mqttserver));
    Serial.println(":" + String(mqttport));
    Serial.print("MQTT ID:");
    Serial.println(mqttid);
    Serial.print("TOPIC SUB:");
    Serial.println(toppicsub);
    Serial.print("TOPIC PUB:");
    Serial.println(toppicpub); 
    Serial.println("_______________________");
}


void callbackMqtt(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Recived from " + String(topic) + " : ");
    for (int i = 0; i < length; i++)
    {
        char temp_data = (char)payload[i];
        buffer_data_from_sever += temp_data;
    }
    Serial.println(buffer_data_from_sever);

    executeMqttCommand(buffer_data_from_sever);
    
    // reset buffer_data_from_sever
    buffer_data_from_sever = "";
}

void setupMQTT()
{  
    Serial.println("Connectting MQTT ...");

    mqtt_client.setServer(mqttserver, mqttport);
    mqtt_client.setCallback(callbackMqtt); // cần 1 hàm callback(topic, payload (nội dung cần truyền), tổng số ký tự có trong payload)

    while (!mqtt_client.connect(mqttid))
    {
        delay(1000);
        statusMQTT();
        Serial.print(".");
    }

    mqtt_client.publish(toppicpub, "Connected !!! Waitting comamd");
    mqtt_client.subscribe(toppicsub);

    showInforMQTT();
}

void loopMQTT()
{
    mqtt_client.loop();
}
#endif
