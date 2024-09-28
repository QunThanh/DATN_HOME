#ifndef __CORE_1_H__
#define __CORE_1_H__

#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

WiFiMulti wifiMulti;

// ======================== wifi ========================
// *** hàm hỗ trợ ***
void showWiFiInfo(){
    IPAddress ip = WiFi.localIP();
    Serial.println();
    Serial.print("[WiFi i4] Connected to ");
    Serial.println(WiFi.SSID());
    Serial.print("[WiFi i4] IP: ");
    Serial.print(ip);
    Serial.print(" (Channel ");
    Serial.print(WiFi.channel());
    Serial.print(" )");
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

// *** hàm chính *** 
void setupWiFi(){
    if (NUM_WIFI <= 0){
        Serial.println("[WiFi err] NUM_WIFI <= 0, check NUM_WIFI again");
        return;
    }

    esp_wifi_stop();

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

    // Debug
    Serial.printf("[WiFi err] WiFi disconnected. Re-connecting now...\n");

    // Try to connect to WiFi
    connectWiFi();
}

#endif