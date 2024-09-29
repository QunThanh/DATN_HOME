#include "configs.h"
#include "network.h"
#include "application.h"

// NOTE: hàm này đã được khai báo ở core1.h
// hàm thu thập thông tin sau đó gửi lên Node-red
void getDataAndSendToNodeRed()
{
    String stringData = "{";

    // nhiệt độ
    stringData += "\"t\": \"";
    stringData += tempData;
    stringData += "\",";

    // độ ẩm
    stringData += "\"h\":\"";
    stringData += humData;
    stringData += "\",";

    // trạng thái led
    stringData += "\"l\":";
    stringData += String(ledStatus);
    stringData += ",";

    // trạng thái quạt
    stringData += "\"f\":";
    stringData += String(fanStatus);
    stringData += ",";
    
    // trạng thái bơm
    stringData += "\"p\":";
    stringData += String(pumpStatus);
    stringData += ",";

    // địa chỉ ip của chip
    stringData += "\"ip\":\"";
    stringData += WiFi.localIP().toString();
    stringData += "\",";

    // thời gian chip đã chạy
    unsigned long now = millis();
    stringData += "\"time\":";
    stringData += String(now);

    stringData += "}";

    // Debug
    Serial.print("[MQTT] send msg to Node-red: ");
    Serial.println(stringData);

    // Publish to MQTT
    boolean success = sendDataToNodeRed(stringData);

    if (success)  return;
    
    // Error
    Serial.println("[MQTT err] Failed to publish to MQTT");
}

// NOTE: hàm này đã được khai báo ở core1.h
// hàm sử lý lệnh từ Node-red gửi xuống
void handleCommandFromNodeRed(String cmd){
    int numIndex = cmd.indexOf(";");

    // kiểm tra có ";" không?
    if (numIndex < 0){
        Serial.println("[handleCmd] cmd missing \";\".");
        return;
    }

    // tách thành 2 phần
    // vd: led;0 (command = led, param = 0);
    String command = cmd.substring(0, numIndex);
    String param = cmd.substring(numIndex + 1);
    int intParam = param.toInt();

    //debug
    Serial.printf("[handleCmd] cmd:\"%s\", pram: \"%d\"\n", command.c_str(), intParam);

    // set trạng thái led
    if (command == "l")
    {
        if (intParam == ledStatus) {
          Serial.printf("[handleCmd] led status nochange !!!\n");
          return;
        }

        // on/off led
        digitalWrite(LED_PIN, intParam);
        // set lại trạng thái led
        ledStatus = intParam;
        // debug
        Serial.printf("[handleCmd] led: %s\n", ledStatus == false ? "off" : "on");
        return;
    }

    // set trạng thái quạt
    if (command == "f")
    {
        if (intParam == fanStatus) {
          Serial.printf("[handleCmd] fan status nochange !!!\n");
          return;
        }

        digitalWrite(FAN_PIN, intParam);
        fanStatus = intParam;
        
        Serial.printf("[handleCmd] fan: %s\n", fanStatus == false ? "off" : "on");
        return;
    }

    // set trạng thái bơm
    if (command == "p")
    {
        if (intParam == pumpStatus) {
          Serial.printf("[handleCmd] pump status nochange !!!\n");
          return;
        }

        digitalWrite(FAN_PIN, intParam);
        pumpStatus = intParam;
        
        Serial.printf("[handleCmd] pump: %s\n", pumpStatus == false ? "off" : "on");
        return;
    }
}

// NOTE: hàm này đã được khai báo ở core0.h
// hàm sử lý nút nhấn.
void handlePressed() {
    // kiểm tra có nút nào nhấn chưa?
    if (!ledPressed && !fanPressed && !pumpPressed)
        return;
    // set trạng thái (đã sử dụng "ngắt" xong)
    isRunningInterrupt = false;
    bool statusCurrent;
    
    // ******** led ********
    if (ledPressed) {
        // đọc trạng thái led hiện tại.
        statusCurrent = digitalRead(LED_PIN); 

        // đảo trạng thái led
        ledStatus = !statusCurrent;
        digitalWrite(LED_PIN, ledStatus); 
        // debug
        Serial.printf("[handlePressed] Led : %s\n", ledStatus == false ? "off" : "on");
        
        // reset trạng thái nút nhấn
        ledPressed = false;   
        return;
    }

    // ******** fan ********
    if (fanPressed) {
        statusCurrent = digitalRead(FAN_PIN); 

        fanStatus = !statusCurrent;
        digitalWrite(FAN_PIN, fanStatus); 
        
        Serial.printf("[handlePressed] Fan : %s\n", fanStatus == false ? "off" : "on");
        
        fanPressed = false;   
        return;
    }

    // ******** pump ********
    if (pumpPressed) {
        statusCurrent = digitalRead(PUMP_PIN); 

        pumpStatus = !statusCurrent;
        digitalWrite(PUMP_PIN, pumpStatus); 
        
        Serial.printf("[handlePressed] Pump : %s\n", pumpStatus == false ? "off" : "on");
        
        pumpPressed = false;   
        return;
    }
}

void setup() {
  Serial.begin(115200);

  setupWiFi();
  delay(50);

  setupMQTT();
  delay(50);
}

void loop() {
  loopWiFi();
  
  loopMqtt();
}
