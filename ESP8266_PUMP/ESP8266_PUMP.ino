#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <DHT.h>


#define NOTI_LED_PIN 2  // don't fix define. don't use D4 and D3
#define IN1_PIN 13      // D7
#define IN2_PIN 12      // D6
#define DOAM_PIN A0     // A0
#define DHT_PIN 5       // D1
// +5V (L289) ->  VIN (ESP)
// GND (L289) ->  GND (ESP)

WiFiClient client;
PubSubClient mqtt_client(client);

//============
const char *ssid = "NTGD";
const char *pass = "112233445566";
const char *mqttserver = "192.168.1.15";  // ip laptop
const int mqttport = 1883;
const char *mqttid = "pump";
const char *toppicsub = "S-ESP8266-PUMP";
const char *toppicpub = "P-ESP8266-PUMP";

unsigned long old_time_report = millis();
unsigned long delay_time_report = 5;  // 5s

float doam = 0;
float nhietdo = 0;
int buffer_bom = 0;
int doam_dat = 0;

String buffer_data_tu_nodered = "";
String buffer_nhietdo = "";
String buffer_doam = "";


//==================
//  SETUP DHT
//==================
DHT dht(DHT_PIN, DHT11);

void setupDHT() {
  dht.begin();
  Serial.println("setup DHT done!");
}

void layGiaTriTuDHT() {
  doam = dht.readHumidity();
  nhietdo = dht.readTemperature();
  doam_dat = analogRead(DOAM_PIN);
  // neu nan thi set ve 0
  // nếu trả về là nan (00.0): báo thiết bị nối sai dây hoặc bị lỗi
  // nêu đúng dữ liệu sẽ trả về 23.7 và 70.0 (23.7 độ C và độ ẩm 70% )
  if (isnan(doam) || isnan(nhietdo)) {
    buffer_nhietdo = "00.0";
    buffer_doam = "00.0";
    return;
  }

  buffer_nhietdo = String(nhietdo);
  buffer_doam = String(doam);
}

//===========
// MQTT
//===========
void xuLyLenhTuNodeRed(String cmd) {
  // pump
  if (cmd == "open8266") {
    digitalWrite(IN1_PIN, HIGH);
    digitalWrite(IN2_PIN, LOW);
    buffer_bom = 10;
    Serial.println("open pump");  // debug
    return;
  }

  if (cmd == "close8266") {
    digitalWrite(IN1_PIN, LOW);
    digitalWrite(IN2_PIN, LOW);
    buffer_bom = 0;
    Serial.println("close pump");  // debug
    return;
  }
}


// hàm lắng nghe sự kiện từ server gửi xuống
void callback(char *topic, byte *payload, unsigned int length) {
  // đọc từ ký tự mà Server gửi xuống.
  for (int i = 0; i < length; i++) {
    char temp_data_mqtt = (char)payload[i];
    buffer_data_tu_nodered += temp_data_mqtt;
  }
  Serial.print("cmd(Server):");
  Serial.print(buffer_data_tu_nodered);
  Serial.println(".");
  xuLyLenhTuNodeRed(buffer_data_tu_nodered);

  // xóa dữ liệu vừa sử lý xong để
  // tránh việc dữ liệu trước bị đè vào dữ liệu sau
  buffer_data_tu_nodered = "";
}

// hàm gửi dữ liệu lên Node-red
bool guiDataLenNodered(String data_gui_nodered) {
  if (mqtt_client.connected()) {
    mqtt_client.publish(toppicpub, data_gui_nodered.c_str());
    return true;
  }
  return false;
}


// reportMQTT
void reportReadings() {

  String dataGuiNodeRed = "{";

  dataGuiNodeRed += "\"dataAuto\":";
  dataGuiNodeRed += String(1);
  dataGuiNodeRed += ",";

  dataGuiNodeRed += "\"bom8266\":";
  dataGuiNodeRed += String(buffer_bom);
  dataGuiNodeRed += ",";

  dataGuiNodeRed += "\"dat8266\":";
  dataGuiNodeRed += String(doam_dat);
  dataGuiNodeRed += ",";

  dataGuiNodeRed += "\"nd8266\":";
  dataGuiNodeRed += String(buffer_nhietdo);
  dataGuiNodeRed += ",";

  dataGuiNodeRed += "\"da8266\":";
  dataGuiNodeRed += String(buffer_doam);
  dataGuiNodeRed += ",";

  unsigned long now = millis();
  dataGuiNodeRed += "\"t\":";
  dataGuiNodeRed += String(now);

  dataGuiNodeRed += "}";

  // Debug
  Serial.println(dataGuiNodeRed);

  // Publish to MQTT
  boolean success = guiDataLenNodered(dataGuiNodeRed);

  // Error
  if (!success) {
    Serial.println("Failed to publish to MQTT");
    return;
  }
}


void setupMQTT() {
  Serial.println("Connecting MQTT");
  mqtt_client.setServer(mqttserver, mqttport);
  mqtt_client.setCallback(callback);

  while (!mqtt_client.connect(mqttid)) {
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
void setupWifi() {
  Serial.println("Connecting WiFi");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected WiFi");
  Serial.print("IP Address:");
  Serial.println(WiFi.localIP());
}

void loopSendReport() {
  if (millis() - old_time_report > delay_time_report * 1000) {
    bool temp_noti = digitalRead(NOTI_LED_PIN);
    digitalWrite(NOTI_LED_PIN, !temp_noti);
    reportReadings();
    old_time_report = millis();
    delay(50);
    return;
  }
  delay(50);
}

void setup() {
  Serial.begin(115200);
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(NOTI_LED_PIN, OUTPUT);
  pinMode(DOAM_PIN, INPUT);

  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, LOW);
  digitalWrite(NOTI_LED_PIN, LOW);

  setupWifi();
  delay(50);

  setupMQTT();
  delay(50);

  setupDHT();
  delay(50);
}

void loop() {
  layGiaTriTuDHT();
  
  loopSendReport();
  mqtt_client.loop();  // hàm duy trì sự kiện lắng nghe lệnh từ Node-red
}