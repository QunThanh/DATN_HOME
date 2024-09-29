#include "configs.h"
#include "core0.h"
#include "core1.h"

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
    // threshold
    // int num_index = cmd.indexOf(";");
    // if(num_index > 0)
    // {
    //   String temp_threshold = String(cmd) + "*";  // thêm * vào cuối lệnh
    //   Serial2.print(temp_threshold);              // gửi đi Slave
    //   Serial.println(temp_threshold);             // debug
    //   return;
    // }

    // // Led phai p.khach
    // if (cmd == "onlppk")
    // {
    //     digitalWrite(LED_PHAI_PKHACH, HIGH);
    //     ttLedPhaiPK = 1;
    //     Serial.println("." + cmd + ".");      // debug
    //     return;
    // }
    // if (cmd == "offlppk")
    // {
    //     digitalWrite(LED_PHAI_PKHACH, LOW);
    //     ttLedPhaiPK = 0;
    //     Serial.println("." + cmd +".");      // debug
    //     return;
    // }

    // // Led trai p.khach
    // if (cmd == "onltpk")
    // {
    //     digitalWrite(LED_TRAI_PKHACH, HIGH);
    //     ttLedTraiPK = 1;
    //     Serial.println("." + cmd + ".");      // debug
    //     return;
    // }
    // if (cmd == "offltpk")
    // {
    //     digitalWrite(LED_TRAI_PKHACH, LOW);
    //     ttLedTraiPK = 0;
    //     Serial.println("." + cmd +".");      // debug
    //     return;
    // }

    // // quat p.khach
    // if (cmd == "onqpk")
    // {
    //     digitalWrite(QUAT_PKHACH, HIGH);
    //     ttQuatPK = 1;
    //     Serial.println("." + cmd +".");      // debug
    //     return;
    // }
    // if (cmd == "offqpk")
    // {
    //     digitalWrite(QUAT_PKHACH, LOW);
    //     ttQuatPK = 0;
    //     Serial.println("." + cmd +".");      // debug
    //     return;
    // }

    // // led p.bep
    // if (cmd == "onlpb")
    // {
    //     digitalWrite(LED_PBEP, HIGH);
    //     ttLedBep = 1;
    //     Serial.println("." + cmd +".");      // debug
    //     return;
    // }
    // if (cmd == "offlpb")
    // {
    //     digitalWrite(LED_PBEP, LOW);
    //     ttLedBep = 0;
    //     Serial.println("." + cmd +".");      // debug
    //     return;
    // }

    // // quat p.bep
    // if (cmd == "onqpb")
    // {
    //     digitalWrite(QUAT_PBEP, HIGH);
    //     ttQuatBep = 1;
    //     Serial.println("." + cmd +".");      // debug
    //     return;
    // }
    // if (cmd == "offqpb")
    // {
    //     digitalWrite(QUAT_PBEP, LOW);
    //     ttQuatBep = 0;
    //     Serial.println("." + cmd +".");      // debug
    //     return;
    // }
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
  
  // pinMode(CB_GAS_PIN, INPUT);
  // pinMode(LED_PHAI_PKHACH, OUTPUT);
  // pinMode(LED_TRAI_PKHACH, OUTPUT);
  // pinMode(QUAT_PKHACH, OUTPUT);
  // pinMode(LED_PBEP, OUTPUT);
  // pinMode(QUAT_PBEP, OUTPUT);

  setupWiFi();
  delay(50);

  setupMQTT();
  delay(50);

  // setupInterrupt();
  // delay(50);

  // setupDHT();
  // delay(50);
}

void loop() {
  // xuLyNutNhan();
  
  // layGiaTriTuDHT();

  // loopSendReport();
  loopWiFi();
  
  loopMqtt();
}
