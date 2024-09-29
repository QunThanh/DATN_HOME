#ifndef __CORE_0_H__
#define __CORE_0_H__

#include <DHT.h>

//==================
//  DHT 11
//==================
DHT dht(DHT_PIN, DHT11);

String humData = "00.0";
String tempData = "00.0";

// ************ hàm hỗ trợ ************
bool getDHT11Data(){
    // lấy dữ liệu tử DHT11
    float hum = dht.readHumidity();
    float temp = dht.readTemperature();
    // nếu trả về là nan (00.0): báo thiết bị nối sai dây hoặc bị lỗi
    if (isnan(hum) || isnan(temp)){
        tempData = "00.0";
        humData = "00.0";
        Serial.println("[DHT err] DHT can't get data.");
        return false;
    }
    
    // nêu đúng dữ liệu sẽ trả về 23.7 và 70.0 (23.7 độ C và độ ẩm 70% )
    tempData = String(temp);
    humData = String(hum);
    return true;
}

// ************ hàm chính ************ 
void setupDHT() {
    dht.begin();
    Serial.println("[DHT] setup DHT done!");
}

//==================
// INTERRUPT
//==================

unsigned long pressLastTime = millis();
bool isRunningInterrupt = false;

bool ledStatus = false;
bool ledPressed = false;

bool fanStatus = false;
bool fanPressed = false;

bool pumpStatus = false;
bool pumpPressed = false;

// ************ hàm hỗ trợ ************
void handlePressed(); // hàm này được viết ở ESP32_2CORE.ino

void IRAM_ATTR pressedLedInterrupt() {
    // trong 500ms thì chỉ nhận 1 lần nhấn
    if (millis() - pressLastTime < PRESS_DEBOUNCE_TIME)
        return;
    pressLastTime = millis();

    // kiểm tra xem có đang sử dụng ngắt chưa?
    if (isRunningInterrupt) return;
    // set trạng thái (đang sử dụng "ngắt")
    isRunningInterrupt = true;  
    // set trạng thái (nút led đã đc nhấn)
    ledPressed = true;
}
void IRAM_ATTR pressedFanInterrupt() {
    if (millis() - pressLastTime < PRESS_DEBOUNCE_TIME)
        return;
    pressLastTime = millis();

    if (isRunningInterrupt) return;
    isRunningInterrupt = true;  
    fanPressed = true;
}
void IRAM_ATTR pressedPumpInterrupt(){
    if (millis() - pressLastTime < PRESS_DEBOUNCE_TIME)
        return;
    pressLastTime = millis();

    if (isRunningInterrupt) return;
    isRunningInterrupt = true;  
    pumpPressed = true;
}

// ************ hàm chính ************ 
void setupInterrupt() {
    pinMode(LED_PIN, INPUT);
    pinMode(FAN_PIN, INPUT);
    pinMode(PUMP_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(LED_PIN), pressedLedInterrupt, FALLING);
    attachInterrupt(digitalPinToInterrupt(FAN_PIN), pressedFanInterrupt, FALLING);
    attachInterrupt(digitalPinToInterrupt(PUMP_PIN), pressedPumpInterrupt, FALLING);
    
    Serial.println("[Interrupt] setup Interrupt done!");
}

#endif