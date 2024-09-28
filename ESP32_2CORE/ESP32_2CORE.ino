#include "configs.h"
#include "core0.h"
#include "core1.h"

// hàm thu thập thông tin sau đó gửi lên Node-red
void getDataAndSendToNodeRed()
{
    String stringData = "{";

    // // nhiet do
    // stringData += "\"nd\": \"";
    // stringData += str_nhietdo;
    // stringData += "\",";

    // // do am
    // stringData += "\"da\":\"";
    // stringData += str_doam;
    // stringData += "\",";

    // //gas
    // stringData += "\"g\":";
    // stringData += String(buffer_cb_gas);
    // stringData += ",";

    // //led phai p.khach
    // stringData += "\"lppk\":";
    // stringData += String(ttLedPhaiPK);
    // stringData += ",";

    // //led trai p.khach
    // stringData += "\"ltpk\":";
    // stringData += String(ttLedTraiPK);
    // stringData += ",";
    
    // //quat p.khach
    // stringData += "\"qpk\":";
    // stringData += String(ttQuatPK);
    // stringData += ",";

    // //led p.bep
    // stringData += "\"lpb\":";
    // stringData += String(ttLedBep);
    // stringData += ",";

    // //quat p.bep
    // stringData += "\"qpb\":";
    // stringData += String(ttQuatBep);
    // stringData += ",";

    // //ip
    // stringData += "\"ip\":\"";
    // stringData += WiFi.localIP().toString();
    // stringData += "\",";

    // unsigned long now = millis();
    // stringData += "\"t\":";
    // stringData += String(now);

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
