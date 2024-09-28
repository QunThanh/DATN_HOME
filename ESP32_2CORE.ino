#include "configs.h"
#include "core0.h"
#include "core1.h"
#include <WiFi.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

#define CB_GAS_PIN              39        
#define DHT_PIN                 17 //TX2         
#define NUT_LED_PHAI_PKHACH     13         
#define NUT_LED_TRAI_PKHACH     12      
#define NUT_QUAT_PKHACH         14 
#define NUT_LED_PBEP            27
#define NUT_QUAT_PBEP           26 
#define LED_PHAI_PKHACH         25    
#define LED_TRAI_PKHACH         33  
#define QUAT_PKHACH             32         
#define LED_PBEP                4        
#define QUAT_PBEP               16  //RX2 


WiFiClient client;
PubSubClient mqtt_client(client);

const char* ssid = "503 QANGMINHHOUSE.VN 0983397152";
const char* pass = "888888889";
const char *mqttserver = "192.168.5.108"; // ip laptop
const int mqttport = 1883;
const char *mqttid = "home";
const char *toppicsub = "S-ESP-HOME";            // gói tin đăng ký với server để lấy tin từ server
const char *toppicpub = "P-ESP-HOME";            // gói tin đăng ký với server để đưuọc gửi lên server

unsigned long old_time_report = millis();
unsigned long delay_time_report = 5;       // 5s 
unsigned long last_time_press = millis();
unsigned long delay_time_press = 500;      // 500ms 

// var interrupt
bool nhanLedPhai = false;
bool nhanLedTrai = false;
bool nhanQuatPK = false;
bool nhanLedBep = false;
bool nhanQuatBep = false;
bool running_interrupt =  false;

//mqtt
String buffer_data_tu_server = "";         // dữ liệu tạm thời lấy từ node-red

//var DHT
float doam = 0;
float nhietdo = 0;
String str_nhietdo = "00.0";
String str_doam = "00.0";

// var handle button
uint8_t ttLedPhaiPK = 0;
uint8_t ttLedTraiPK = 0;
uint8_t ttQuatPK = 0;
uint8_t ttLedBep = 0;
uint8_t ttQuatBep = 0;
bool temp_IO_value = 0;

//==================
//  SETUP DHT
//==================
DHT dht(DHT_PIN, DHT11);

void setupDHT()
{
    dht.begin();
    Serial.println("setup DHT done!");
}

void layGiaTriTuDHT()
{
    doam = dht.readHumidity();
    nhietdo = dht.readTemperature();
    // neu nan thi set ve 0
    // nếu trả về là nan (00.0): báo thiết bị nối sai dây hoặc bị lỗi
    // nêu đúng dữ liệu sẽ trả về 23.7 và 70.0 (23.7 độ C và độ ẩm 70% )
    if (isnan(doam) || isnan(nhietdo))
      {
        str_nhietdo = "00.0";
        str_doam = "00.0";
        return;
      }
      
    str_nhietdo = String(nhietdo);
    str_doam = String(doam);
}

//==================
// SETUP Interrupt : ngắt phát hiện nút nhấn
//==================
// p.khach
void IRAM_ATTR xuly_nut_led_phai_pkhach()
{
  if (millis() - last_time_press < delay_time_press)
    return;
  last_time_press = millis();

  if (running_interrupt) return;
  running_interrupt = true;  
  nhanLedPhai = true;
}
void IRAM_ATTR xuly_nut_led_trai_pkhach()
{
  if (millis() - last_time_press < delay_time_press)
    return;
  last_time_press = millis();

  if (running_interrupt) return;
  running_interrupt = true;  
  nhanLedTrai = true;
}
void IRAM_ATTR xuly_nut_quat_pkhach()
{
  if (millis() - last_time_press < delay_time_press)
    return;
  last_time_press = millis();

  if (running_interrupt) return;
  running_interrupt = true;  
  nhanQuatPK = true;
}

// p.bep
void IRAM_ATTR xuly_nut_led_pbep()
{
  if (millis() - last_time_press < delay_time_press)
    return;
  last_time_press = millis();

  if (running_interrupt) return;
  running_interrupt = true;  
  nhanLedBep = true;
}
void IRAM_ATTR xuly_nut_quat_pbep()
{
  if (millis() - last_time_press < delay_time_press)
    return;
  last_time_press = millis();
  
  if (running_interrupt) return;
  running_interrupt = true;  
  nhanQuatBep = true;
}

void setupInterrupt()
{
  // p.khach
  pinMode(NUT_LED_PHAI_PKHACH, INPUT_PULLUP);
  pinMode(NUT_LED_TRAI_PKHACH, INPUT_PULLUP);
  pinMode(NUT_QUAT_PKHACH, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(NUT_LED_PHAI_PKHACH), xuly_nut_led_phai_pkhach, FALLING); //RISING
  attachInterrupt(digitalPinToInterrupt(NUT_LED_TRAI_PKHACH), xuly_nut_led_trai_pkhach, FALLING);
  attachInterrupt(digitalPinToInterrupt(NUT_QUAT_PKHACH), xuly_nut_quat_pkhach, FALLING);
  
  //p.bep
  pinMode(NUT_LED_PBEP, INPUT_PULLUP);
  pinMode(NUT_QUAT_PBEP, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(NUT_LED_PBEP), xuly_nut_led_pbep, FALLING);
  attachInterrupt(digitalPinToInterrupt(NUT_QUAT_PBEP), xuly_nut_quat_pbep, FALLING);
  Serial.println("setup Interrupt done!");
}

//==================
// xu ly nut nhan
//==================
void xuLyNutNhan()
{
  // kiểm tra có nút nào nhấn chưa?  nếu rồi thì xuống dưới thực hiện không thì bỏ qua hàm này
  if (!nhanLedPhai && !nhanLedTrai && !nhanQuatPK && !nhanLedBep && !nhanQuatBep)
    return;
  running_interrupt = false;
  // led phai p.khach
  if (nhanLedPhai)
  {
    temp_IO_value = digitalRead(LED_PHAI_PKHACH);         // đọc trạng thái của máy bơm hiện tại.
    ttLedPhaiPK = !temp_IO_value;
    digitalWrite(LED_PHAI_PKHACH, ttLedPhaiPK);        // đảo trạng thái máy bơm
    
    Serial.println("ledphai");
    nhanLedPhai = false;                  // reset trạng thái nút nhấn
    return;
  }

  // led trai p.khach
  if (nhanLedTrai)
  {
    temp_IO_value = digitalRead(LED_TRAI_PKHACH);         // đọc trạng thái của máy bơm hiện tại.
    ttLedTraiPK = !temp_IO_value;
    digitalWrite(LED_TRAI_PKHACH, ttLedTraiPK);        // đảo trạng thái máy bơm
    
    Serial.println("ledtrai");
    nhanLedTrai = false;                  // reset trạng thái nút nhấn
    return;
  }  

  // quat p.khach  
  if (nhanQuatPK)
  {
    temp_IO_value = digitalRead(QUAT_PKHACH);           // đọc trạng thái của led hiện tại.
    ttQuatPK = !temp_IO_value;
    digitalWrite(QUAT_PKHACH, ttQuatPK);          // đảo trạng thái led
    
    Serial.println("quatkhach"); 
    nhanQuatPK = false;                   // reset trạng thái nút nhấn
    return;
  }

  // led p.bep
  if (nhanLedBep)
  {
    temp_IO_value = digitalRead(LED_PBEP);           // đọc trạng thái của led hiện tại.
    ttLedBep = !temp_IO_value;
    digitalWrite(LED_PBEP, ttLedBep);          // đảo trạng thái led

    Serial.println("ledbep"); 
    nhanLedBep = false;                   // reset trạng thái nút nhấn
    return;
  }

  // quat p.bep  
  if (nhanQuatBep)
  {
    temp_IO_value = digitalRead(QUAT_PBEP);           // đọc trạng thái của led hiện tại.
    ttQuatBep = !temp_IO_value;
    digitalWrite(QUAT_PBEP, ttQuatBep);          // đảo trạng thái led
    
    Serial.println("quatbep"); 
    nhanQuatBep = false;                   // reset trạng thái nút nhấn
    return;
  }  

}


//===========
// MQTT
//===========
void xuLyLenhTuNodeRed(String cmd)
{
  // threshold
  // int num_index = cmd.indexOf(";");
  // if(num_index > 0)
  // {
  //   String temp_threshold = String(cmd) + "*";  // thêm * vào cuối lệnh
  //   Serial2.print(temp_threshold);              // gửi đi Slave
  //   Serial.println(temp_threshold);             // debug
  //   return;
  // }

  // Led phai p.khach
  if (cmd == "onlppk")
  {
    digitalWrite(LED_PHAI_PKHACH, HIGH);
    ttLedPhaiPK = 1;
    Serial.println("." + cmd + ".");      // debug
    return;
  }
  if (cmd == "offlppk")
  {
    digitalWrite(LED_PHAI_PKHACH, LOW);
    ttLedPhaiPK = 0;
    Serial.println("." + cmd +".");      // debug
    return;
  }

  // Led trai p.khach
  if (cmd == "onltpk")
  {
    digitalWrite(LED_TRAI_PKHACH, HIGH);
    ttLedTraiPK = 1;
    Serial.println("." + cmd + ".");      // debug
    return;
  }
  if (cmd == "offltpk")
  {
    digitalWrite(LED_TRAI_PKHACH, LOW);
    ttLedTraiPK = 0;
    Serial.println("." + cmd +".");      // debug
    return;
  }

  // quat p.khach
  if (cmd == "onqpk")
  {
    digitalWrite(QUAT_PKHACH, HIGH);
    ttQuatPK = 1;
    Serial.println("." + cmd +".");      // debug
    return;
  }
  if (cmd == "offqpk")
  {
    digitalWrite(QUAT_PKHACH, LOW);
    ttQuatPK = 0;
    Serial.println("." + cmd +".");      // debug
    return;
  }

  // led p.bep
  if (cmd == "onlpb")
  {
    digitalWrite(LED_PBEP, HIGH);
    ttLedBep = 1;
    Serial.println("." + cmd +".");      // debug
    return;
  }
  if (cmd == "offlpb")
  {
    digitalWrite(LED_PBEP, LOW);
    ttLedBep = 0;
    Serial.println("." + cmd +".");      // debug
    return;
  }

  // quat p.bep
  if (cmd == "onqpb")
  {
    digitalWrite(QUAT_PBEP, HIGH);
    ttQuatBep = 1;
    Serial.println("." + cmd +".");      // debug
    return;
  }
  if (cmd == "offqpb")
  {
    digitalWrite(QUAT_PBEP, LOW);
    ttQuatBep = 0;
    Serial.println("." + cmd +".");      // debug
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
    buffer_data_tu_server += temp_data_mqtt;  
  }
  Serial.print("cmd Node-red (Master):");
  Serial.print(buffer_data_tu_server);
  Serial.println(".");
  xuLyLenhTuNodeRed(buffer_data_tu_server);
  
  // xóa dữ liệu vừa sử lý xong để 
  // tránh việc dữ liệu trước bị đè vào dữ liệu sau
  buffer_data_tu_server = "";
}

// reportMQTT
void reportReadings()
{
    int buffer_cb_gas = analogRead(CB_GAS_PIN);
    String dataGuiServer = "{";

    // nhiet do
    dataGuiServer += "\"nd\": \"";
    dataGuiServer += str_nhietdo;
    dataGuiServer += "\",";

    // do am
    dataGuiServer += "\"da\":\"";
    dataGuiServer += str_doam;
    dataGuiServer += "\",";

    //gas
    dataGuiServer += "\"g\":";
    dataGuiServer += String(buffer_cb_gas);
    dataGuiServer += ",";

    //led phai p.khach
    dataGuiServer += "\"lppk\":";
    dataGuiServer += String(ttLedPhaiPK);
    dataGuiServer += ",";

    //led trai p.khach
    dataGuiServer += "\"ltpk\":";
    dataGuiServer += String(ttLedTraiPK);
    dataGuiServer += ",";
    
    //quat p.khach
    dataGuiServer += "\"qpk\":";
    dataGuiServer += String(ttQuatPK);
    dataGuiServer += ",";

    //led p.bep
    dataGuiServer += "\"lpb\":";
    dataGuiServer += String(ttLedBep);
    dataGuiServer += ",";

    //quat p.bep
    dataGuiServer += "\"qpb\":";
    dataGuiServer += String(ttQuatBep);
    dataGuiServer += ",";

    //ip
    dataGuiServer += "\"ip\":\"";
    dataGuiServer += WiFi.localIP().toString();
    dataGuiServer += "\",";

    unsigned long now = millis();
    dataGuiServer += "\"t\":";
    dataGuiServer += String(now);

    dataGuiServer += "}";

    // Debug
    Serial.println(dataGuiServer);

    // Publish to MQTT
    boolean success = guiDataLenNodered(dataGuiServer);

    if (success)  return;
    
    // Error
    Serial.println("Failed to publish to MQTT");
}

// hàm gửi dữ liệu lên Node-red
bool guiDataLenNodered( String data_gui_nodered )
{
  if (mqtt_client.connected())
  {
    mqtt_client.publish(toppicpub, data_gui_nodered.c_str());
    return true;
  }
  return false;
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


void loopSendReport(){
  if ( millis() - old_time_report > delay_time_report * 1000)
  {
    reportReadings();
    old_time_report = millis();
    delay(50); 
    return;
  }
  delay(50); 
}

void setup() {
  Serial.begin(115200);
  
  pinMode(CB_GAS_PIN, INPUT);
  pinMode(LED_PHAI_PKHACH, OUTPUT);
  pinMode(LED_TRAI_PKHACH, OUTPUT);
  pinMode(QUAT_PKHACH, OUTPUT);
  pinMode(LED_PBEP, OUTPUT);
  pinMode(QUAT_PBEP, OUTPUT);

  setupWifi();
  delay(50);

  setupMQTT();
  delay(50);

  setupInterrupt();
  delay(50);

  setupDHT();
  delay(50);
}

void loop() {

  mqtt_client.loop(); 

  xuLyNutNhan();
  
  layGiaTriTuDHT();

  loopSendReport();
}
