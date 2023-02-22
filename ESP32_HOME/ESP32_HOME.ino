#include <SPI.h>
#include <WiFi.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

#define CB_GAS_PIN              33         
#define DHT_PIN                 25         
#define NUT_LED_PHAI_PKHACH     26         
#define NUT_LED_TRAI_PKHACH     26 //     
#define NUT_LED_PBEP            27
#define NUT_QUAT_PKHACH         13 
#define NUT_QUAT_PBEP           13 //
#define LED_PHAI_PKHACH         32    
#define LED_TRAI_PKHACH         32 // 
#define LED_PBEP                14        
#define QUAT_PKHACH             12         
#define QUAT_PBEP               12 //  



WiFiClient client;
PubSubClient mqtt_client(client);

const char *ssid = "NTGD";   // tên mạng mà máy tính kết nối
const char *pass = "112233445566";              // mật khẩu mạng
const char *mqttserver = "192.168.1.7";   // địa chỉ mạng của máy tính
const int mqttport = 1883;
const char *mqttid = "home";
const char *toppicsub = "S-ESP-HOME";            // gói tin đăng ký với server để lấy tin từ server
const char *toppicpub = "P-ESP-HOME";            // gói tin đăng ký với server để đưuọc gửi lên server

unsigned long old_time_report = millis();
unsigned long delay_time_report = 5;       // 5s 

// var interrupt
bool nhan_nut_led_phai_pkhach = false;
bool nhan_nut_led_trai_pkhach = false;
bool nhan_nut_quat_pkhach = false;
bool nhan_nut_led_pbep = false;
bool nhan_nut_quat_pbep = false;

//mqtt
String buffer_data_tu_server = "";         // dữ liệu tạm thời lấy từ node-red

//var DHT
float doam = 0;
float nhietdo = 0;
String str_nhietdo = "00.0";
String str_doam = "00.0";

// var handle button
uint8_t tt_led_phai_pkhach = 0;
uint8_t tt_led_trai_pkhach = 0;
uint8_t tt_quat_pkhach = 0;
uint8_t tt_led_pbep = 0;
uint8_t tt_quat_pbep = 0;

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
  nhan_nut_led_phai_pkhach = true;
}
void IRAM_ATTR xuly_nut_led_trai_pkhach()
{
  nhan_nut_led_trai_pkhach = true;
}
void IRAM_ATTR xuly_nut_quat_pkhach()
{
  nhan_nut_quat_pkhach = true;
}

// p.bep
void IRAM_ATTR xuly_nut_led_pbep()
{
  nhan_nut_led_pbep = true;
}
void IRAM_ATTR xuly_nut_quat_pbep()
{
  nhan_nut_quat_pbep = true;
}

void setupInterrupt()
{
  // p.khach
  pinMode(NUT_LED_PHAI_PKHACH, INPUT);
  pinMode(NUT_LED_TRAI_PKHACH, INPUT);
  pinMode(NUT_QUAT_PKHACH, INPUT);
  attachInterrupt(digitalPinToInterrupt(NUT_LED_PHAI_PKHACH), xuly_nut_led_phai_pkhach, RISING);
  attachInterrupt(digitalPinToInterrupt(NUT_LED_TRAI_PKHACH), xuly_nut_led_trai_pkhach, RISING);
  attachInterrupt(digitalPinToInterrupt(NUT_QUAT_PKHACH), xuly_nut_quat_pkhach, RISING);
  
  //p.bep
  pinMode(NUT_LED_PBEP, INPUT);
  pinMode(NUT_QUAT_PBEP, INPUT);
  attachInterrupt(digitalPinToInterrupt(NUT_LED_PBEP), xuly_nut_led_pbep, RISING);
  attachInterrupt(digitalPinToInterrupt(NUT_QUAT_PBEP), xuly_nut_quat_pbep, RISING);
  Serial.println("setup Interrupt done!");
}

//==================
// xu ly nut nhan
//==================
void xuLyNutNhan()
{
  // kiểm tra có nút nào nhấn chưa?  nếu rồi thì xuống dưới thực hiện không thì bỏ qua hàm này
  if (!nhan_nut_led_phai_pkhach && !nhan_nut_led_trai_pkhach && !nhan_nut_quat_pkhach && !nhan_nut_led_pbep && !nhan_nut_quat_pbep)
    return;
  
  // led phai p.khach
  if (nhan_nut_led_phai_pkhach)
  {
    tt_led_phai_pkhach = digitalRead(LED_PHAI_PKHACH);         // đọc trạng thái của máy bơm hiện tại.
    digitalWrite(LED_PHAI_PKHACH, !tt_led_phai_pkhach);        // đảo trạng thái máy bơm
    
    tt_led_phai_pkhach = !tt_led_phai_pkhach;
    nhan_nut_led_phai_pkhach = false;                  // reset trạng thái nút nhấn
    return;
  }

  // led trai p.khach
  if (nhan_nut_led_trai_pkhach)
  {
    tt_led_trai_pkhach = digitalRead(LED_TRAI_PKHACH);         // đọc trạng thái của máy bơm hiện tại.
    digitalWrite(LED_TRAI_PKHACH, !tt_led_trai_pkhach);        // đảo trạng thái máy bơm
    
    tt_led_trai_pkhach = !tt_led_trai_pkhach;
    nhan_nut_led_trai_pkhach = false;                  // reset trạng thái nút nhấn
    return;
  }  

  // quat p.khach  
  if (nhan_nut_quat_pkhach)
  {
    tt_quat_pkhach = digitalRead(QUAT_PKHACH);           // đọc trạng thái của led hiện tại.
    digitalWrite(QUAT_PKHACH, !tt_quat_pkhach);          // đảo trạng thái led
    
    tt_quat_pkhach = !tt_quat_pkhach;
    nhan_nut_quat_pkhach = false;                   // reset trạng thái nút nhấn
    return;
  }

  // led p.bep
  if (nhan_nut_led_pbep)
  {
    tt_led_pbep = digitalRead(LED_PBEP);           // đọc trạng thái của led hiện tại.
    digitalWrite(LED_PBEP, !tt_led_pbep);          // đảo trạng thái led
    tt_led_pbep = !tt_led_pbep;
    nhan_nut_led_pbep = false;                   // reset trạng thái nút nhấn
    return;
  }

  // quat p.bep  
  if (nhan_nut_quat_pbep)
  {
    tt_quat_pbep = digitalRead(QUAT_PBEP);           // đọc trạng thái của led hiện tại.
    digitalWrite(QUAT_PBEP, !tt_quat_pbep);          // đảo trạng thái led
    
    tt_quat_pbep = !tt_quat_pbep;
    nhan_nut_quat_pbep = false;                   // reset trạng thái nút nhấn
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
  if (cmd == "onledphaikhach")
  {
    digitalWrite(LED_PHAI_PKHACH, HIGH);
    tt_led_phai_pkhach = 1;
    Serial.println("." + cmd + ".");      // debug
    return;
  }
  if (cmd == "offledphaikhach")
  {
    digitalWrite(LED_PHAI_PKHACH, LOW);
    tt_led_phai_pkhach = 0;
    Serial.println("." + cmd +".");      // debug
    return;
  }

  // Led trai p.khach
  if (cmd == "onledtraikhach")
  {
    digitalWrite(LED_TRAI_PKHACH, HIGH);
    tt_led_trai_pkhach = 1;
    Serial.println("." + cmd + ".");      // debug
    return;
  }
  if (cmd == "offledtraikhach")
  {
    digitalWrite(LED_TRAI_PKHACH, LOW);
    tt_led_trai_pkhach = 0;
    Serial.println("." + cmd +".");      // debug
    return;
  }

  // quat p.khach
  if (cmd == "onquatkhach")
  {
    digitalWrite(QUAT_PKHACH, HIGH);
    tt_quat_pkhach = 1;
    Serial.println("." + cmd +".");      // debug
    return;
  }
  if (cmd == "offquatkhach")
  {
    digitalWrite(QUAT_PKHACH, LOW);
    tt_quat_pkhach = 0;
    Serial.println("." + cmd +".");      // debug
    return;
  }

  // led p.bep
  if (cmd == "onledbep")
  {
    digitalWrite(LED_PBEP, HIGH);
    tt_led_pbep = 1;
    Serial.println("." + cmd +".");      // debug
    return;
  }
  if (cmd == "offledbep")
  {
    digitalWrite(LED_PBEP, LOW);
    tt_led_pbep = 0;
    Serial.println("." + cmd +".");      // debug
    return;
  }

  // quat p.bep
  if (cmd == "onquatbep")
  {
    digitalWrite(QUAT_PBEP, HIGH);
    tt_quat_pbep = 1;
    Serial.println("." + cmd +".");      // debug
    return;
  }
  if (cmd == "offquatbep")
  {
    digitalWrite(QUAT_PBEP, LOW);
    tt_quat_pbep = 0;
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
    dataGuiServer += "\"nd\":";
    dataGuiServer += str_nhietdo;
    dataGuiServer += ",";

    // do am
    dataGuiServer += "\"da\":";
    dataGuiServer += str_doam;
    dataGuiServer += ",";

    //gas
    dataGuiServer += "\"g\":";
    dataGuiServer += String(buffer_cb_gas);
    dataGuiServer += ",";

    //led phai p.khach
    dataGuiServer += "\"lppk\":";
    dataGuiServer += String(tt_led_phai_pkhach);
    dataGuiServer += ",";

    //led trai p.khach
    dataGuiServer += "\"ltpk\":";
    dataGuiServer += String(tt_led_trai_pkhach);
    dataGuiServer += ",";
    
    //quat p.khach
    dataGuiServer += "\"qpk\":";
    dataGuiServer += String(tt_quat_pkhach);
    dataGuiServer += ",";

    //led p.bep
    dataGuiServer += "\"lpb\":";
    dataGuiServer += String(tt_led_pbep);
    dataGuiServer += ",";

    //quat p.bep
    dataGuiServer += "\"qpb\":";
    dataGuiServer += String(tt_quat_pbep);
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
  
  loopSendReport();
}
