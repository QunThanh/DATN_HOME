#ifndef __CORE_0_H__
#define __CORE_0_H__

#include <DHT.h>

//==================
//  SETUP DHT
//==================
DHT dht(DHT_PIN, DHT11);

String humData = "00.0";
String tempData = "00.0";

// ************ hàm hỗ trợ ************
bool getDHT11Data(){
    float hum = dht.readHumidity();
    float temp = dht.readTemperature();
    // neu nan thi set ve 0
    // nếu trả về là nan (00.0): báo thiết bị nối sai dây hoặc bị lỗi
    // nêu đúng dữ liệu sẽ trả về 23.7 và 70.0 (23.7 độ C và độ ẩm 70% )
    if (isnan(hum) || isnan(temp)){
        tempData = "00.0";
        humData = "00.0";
        Serial.println("[DHT err] DHT can't get data.");
        return false;
    }
    
    tempData = String(temp);
    humData = String(hum);
    return true;
}

// ************ hàm chính ************ 
void setupDHT()
{
    dht.begin();
    Serial.println("[DHT] setup DHT done!");
}


#endif