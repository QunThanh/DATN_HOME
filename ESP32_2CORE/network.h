#ifndef __CORE_1_H__
#define __CORE_1_H__

#include <WiFi.h>
#include <WiFiMulti.h>
#include <PubSubClient.h>
#include <WiFiClient.h>


//==================
// WIFI
//==================

WiFiMulti wifiMulti;

// ************ hàm hỗ trợ ************
void showWiFiInfo(){
    IPAddress ip = WiFi.localIP();
    Serial.println();
    Serial.print("[WiFi] Connected to \"");
    Serial.print(WiFi.SSID());
    Serial.println("\"");
    Serial.print("[WiFi] IP: ");
    Serial.print(ip);
    Serial.print(" (Channel ");
    Serial.print(WiFi.channel());
    Serial.println(")");
}

bool isConnectedWiFi(){
    return WiFi.status() == WL_CONNECTED;
}

void connectWiFi(){
    Serial.printf("\n\n[WiFi] Connecting to WiFi...\n");
    Serial.printf("[WiFi] ");

    // thử kết nối mạng trong 10s
    // quá 10s thì reset chip
    int tries = 0;
    wifiMulti.run();
    while (WiFi.status() != WL_CONNECTED)
    {
        wifiMulti.run();
        
        delay(500);   Serial.print(".");
        delay(500);   Serial.print("_");
        
        if (tries > 10){
            Serial.printf("\n\n[WiFi err] REBOOTING after %d times connecting to WiFi...\n\n", tries);
            delay(500);
            // reset chip
            ESP.restart();      
            return;
        }

        tries++;

        // reset
        if (tries > 99)
        tries = 0;
    }
    Serial.println("");

    // hiển thị thông tin WiFi
    showWiFiInfo();
}

// ************ hàm chính ************ 
void setupWiFi(){
    if (NUM_WIFI <= 0){
        Serial.println("[WiFi err] NUM_WIFI <= 0, check NUM_WIFI again");
        return;
    }

    // thêm thông tin vô WiFi Multi
    for (byte i=0; i < NUM_WIFI; i++)
        wifiMulti.addAP(wifi_ssid[i], wifi_pw[i]);

    // kết nối wifi
    connectWiFi();
}
void loopWiFi()
{
    // kiểm tra tổng số wf
    if (NUM_WIFI <= 0){
        Serial.println("[WiFi err] NUM_WIFI <= 0, check NUM_WIFI again");
        return;
    }

    // kiểm tra kết nối mạng định kì
    if (isConnectedWiFi()) return;

    Serial.printf("[WiFi err] WiFi disconnected. Re-connecting now...\n");

    // kết nối lại WiFi
    connectWiFi();
}
//==================
// MQTT
//==================

WiFiClient client;
PubSubClient mqttClient(client);

unsigned long mqttLastConnectedTime = 0;
// ************ hàm hỗ trợ ************
void getDataAndSendToNodeRed();             // hàm này được viết ở ESP32_2CORE.ino
void handleCommandFromNodeRed(String cmd);  // hàm này được viết ở ESP32_2CORE.ino

bool sendDataToNodeRed(String stringData){
    if (mqttClient.connected()){
        mqttClient.publish(TOPPIC_PUB, stringData.c_str());
        return true;
    }
    return false;
}


// ************ hàm callback ************
void callback(char *topic, byte *payload, unsigned int len)
{
    // viết ký hiệu hết chuỗi vào cuối chuỗi.
    payload[len] = '\0';
    // chuyển chuỗi thành string
    String strPayload = String((char*)payload);
    
    Serial.printf("[MQTT callback] Server send: %s\n,", strPayload.c_str());
    delay(10);
    handleCommandFromNodeRed(strPayload);
}

// ************ hàm chính ************
void setupMQTT() {
    Serial.println("[MQTT] Connectting MQTT");
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback(callback);
    Serial.printf("[MQTT] MQTT Server: %s:%d\n", MQTT_SERVER, MQTT_PORT);
    Serial.printf("[MQTT] MQTT Subscribe %s\n", TOPPIC_SUB);
    Serial.printf("[MQTT] MQTT Publish: %s\n", TOPPIC_PUB);
}

void loopMqtt() {   
    unsigned long delta = millis() - mqttLastConnectedTime;
    
    // kiểm tra kết nối MQTT
    if (mqttClient.connected()){
        // hàm duy trì nhận lệnh từ Node-red
        mqttClient.loop();
        
        // < 30s bỏ hàm này
        if (mqttLastConnectedTime > 0 && delta <= (MQTT_SEND_DATA_TIME * 1000))
            return;
        
        // 30s gửi lên Node-red 1 lần
        mqttLastConnectedTime = millis();
        getDataAndSendToNodeRed();
        return;
    }
    
    // Nếu MQTT mất kết nối. Chờ 30s
    if (mqttLastConnectedTime > 0 && delta <= (MQTT_RECONNECT_TIME * 1000))
        return;
    
    Serial.printf("[MQTT] MQTT Connecting...\n");
    
    // thử kết nối lại MQTT
    if (mqttClient.connect(MQTT_ID)) {
        Serial.printf("[MQTT] MQTT Connected\n");
        mqttLastConnectedTime = millis();
        
        // đăng ký topic trên Server
        mqttClient.subscribe(TOPPIC_SUB);

        Serial.println("[MQTT] Connected MQTT");

        // gửi gói tin lần đầu tiên lên server
        getDataAndSendToNodeRed();
        return;
    }

    // kết nối mqtt thất bại, chờ 30s kết nối lại
    Serial.print(F("[MQTT err] MQTT failed to connect, rc="));
    Serial.println(mqttClient.state());
}

//==================
// SETUP CORE
//==================
TaskHandle_t NetworkHandler;

// ************ function core ************

void setupNetwork() {
    setupWiFi();
    delay(50);

    setupMQTT();
    delay(50);
}

void loopNetwork() {
    loopWiFi();
    loopMqtt();
}


void NetworkFuncCode( void * pvParameters )
{
    delay(100);
    Serial.printf("[Multitasking] Running NetworkFuncCode() on Core %i\n", xPortGetCoreID());

    setupNetwork();  

    while (true)
    {
        loopNetwork();
    }
}

// ************ hàm chính ************
void setupNetworkCore() {
    int result = xTaskCreate(
                    NetworkFuncCode,    /* task function */
                    "network",          /* name of task */
                    4096,             /* stack size of task */
                    NULL,             /* parameter of the task */
                    tskIDLE_PRIORITY, /* priority of the task, 0 = tskIDLE_PRIORITY is lowest priority */
                    &NetworkHandler);   /* task handle/pointer to keep track of created task */

    if (result)     Serial.printf("[Multitasking] Task network created: %i\n", result);
    else            Serial.printf("[Multitasking] Failed to create Task 2 %i\n", result);
}


#endif