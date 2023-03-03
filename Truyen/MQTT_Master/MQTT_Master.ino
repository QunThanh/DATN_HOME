#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

WiFiClient client;
PubSubClient mqtt_client(client);

const char *ssid = "Homestay duy anh T4";   // tên mạng mà máy tính kết nối
const char *pass = "88888888";              // mật khẩu mạng
const char *mqttserver = "192.168.0.107";   // địa chỉ mạng của máy tính
const int mqttport = 1883;
const char *mqttid = "master";
const char *toppicsub = "S-ESP";            // gói tin đăng ký với server để lấy tin từ server
const char *toppicpub = "P-ESP";            // gói tin đăng ký với server để đưuọc gửi lên server

String buffer_data_tu_nodered = "";         // dữ liệu tạm thời lấy từ node-red
String buffer_data_tu_slave = "";           // dữ liệu tạm thời lấy từ ESP-Slave
String data_tu_Slave_gui_len = "";          // dữ liệu từ ESP-Slave gửi lên
String temp_data = "";                      // dữ liệu tạm thời nhận từ UART 

//===========
// MQTT
//===========
void xuLyLenhTuNodeRed(String cmd)
{
  //threshold
  int num_index = cmd.indexOf(";");
  if(num_index > 0)
  {
    String temp_threshold = String(cmd) + "*";  // thêm * vào cuối lệnh
    Serial2.print(temp_threshold);              // gửi đi Slave
    Serial.println(temp_threshold);             // debug
    return;
  }

  // Led
  if (cmd == "onled")
  {
    Serial2.print("onled*");       // gửi đi Slave
    Serial.println("onled*");      // debug
    return;
  }

  if (cmd == "offled")
  {
    Serial2.print("offled*");      // gửi đi Slave
    Serial.println("offled*");     // debug
    return;
  }
  
  //pump 
  if (cmd == "onpump")
  {
    Serial2.print("onpump*");    // gửi đi Slave
    Serial.println("onpump*");     // debug
    return;
  }

  if (cmd == "offpump")
  {
    Serial2.print("offpump*");    // gửi đi Slave
    Serial.println("offpump*");     // debug
    return;
  }

  //auto 
  if (cmd == "auto")
  {
    Serial2.print("auto*");    // gửi đi Slave
    Serial.println("auto*");     // debug
    return;
  }

  if (cmd == "manual")
  {
    Serial2.print("manual*");    // gửi đi Slave
    Serial.println("manual*");     // debug
    return;
  }
}

// hàm lắng nghe sự kiện từ server gửi xuống
void callback(char *topic, byte *payload, unsigned int length)
{
  // đọc từ ký tự mà Server gửi xuống.
  for (int i = 0; i < length; i++)
  {
    char temp_data_mqtt = (char)payload[i];
    buffer_data_tu_nodered += temp_data_mqtt;  
  }
  Serial.print("cmd Node-red (Master): ");
  Serial.println(buffer_data_tu_nodered);
  xuLyLenhTuNodeRed(buffer_data_tu_nodered);
  
  // xóa dữ liệu vừa sử lý xong để 
  // tránh việc dữ liệu trước bị đè vào dữ liệu sau
  buffer_data_tu_nodered = "";
}

// hàm gửi dữ liệu lên Node-red
void guiDataLenNodered( String data_gui_nodered )
{
  if (mqtt_client.connected())
  {
    mqtt_client.publish(toppicpub, data_gui_nodered.c_str());
  }
}

void setupMQTT()
{
  Serial.println("Connectting MQTT");
  mqtt_client.setServer(mqttserver, mqttport);
  mqtt_client.setCallback(callback);             

  while (!mqtt_client.connect(mqttid))
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected MQTT");
  mqtt_client.publish(toppicpub, "Đã kết nối và chờ lệnh");
  mqtt_client.subscribe(toppicsub);
}


//===========
// Wifi
//===========
void setupWifi()
{
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected WiFi");
  Serial.print("IP Address:");
  Serial.println(WiFi.localIP());
}


//====================================
// nhận tin từ Slave gửi bằng UART
//====================================
void docDataTuSlave()
{
  while (Serial2.available())
  {
    char incomingString = char(Serial2.read());    // đọc từng byte
    if (incomingString != '*')
    {
      temp_data += incomingString;
    }
    else
    {
      data_tu_Slave_gui_len = temp_data;
      Serial.print("data from Slave (Master): ");
      Serial.println(data_tu_Slave_gui_len);
      temp_data = "";                              // reset dữ liệu vừa đọc

      guiDataLenNodered(data_tu_Slave_gui_len);    // gửi data lên Node-red
      data_tu_Slave_gui_len = "";                  // reset dữ liệu vừa lưu
    }
  }
}

//==========
// RUN
//==========
void setup()
{
  Serial2.begin(9600);      // serial để truyền tin
  Serial.begin(9600);       // serial để in ra để kiểm tra lỗi

  setupWifi();              
  delay(50);

  setupMQTT();
  delay(50);
}

void loop()
{
  mqtt_client.loop();       // hàm duy trì sự kiện lắng nghe lệnh từ Node-red

  docDataTuSlave();         // hàm đọc và gửi dữ liệu lên node-red
  delay(400);               // phải để delay tối thiểu là 200ms để tránh bị đọc sai dữ liệu nhận từ ESP-slave 
}