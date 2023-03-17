#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

// only input 35,34,39,36
#define NOTI_LED_PIN 2      // don't fix define. don't use D2, TX0 and RX0
#define BTN_PUM32_PIN 26    // G26
#define BTN_LED32_PIN 27    // G27
#define BTN_PUM8266_PIN 14  // G14
#define BTN_CHANNEL_PIN 12  // G12
#define LED_CHANNEL_PIN 13  // G13
// 5V (tổ ong hoặc nguồn 5V) nối chân 5V
// GND (tổ ong hoặc nguồn 5V) nối chân GND

WiFiClient client;
PubSubClient mqtt_client(client);

//============
const char *ssid = "NTGD";
const char *pass = "112233445566";
const char *mqttserver = "192.168.1.15";  // ip laptop
const int mqttport = 1883;
const char *mqttid = "control";
const char *toppicsub = "S-ESP32-CONTROL";
const char *toppicpub = "P-ESP32-CONTROL";

unsigned long old_time_report = millis();
unsigned long delay_time_report = 5;  // 5s

String buffer_data_tu_nodered = "";
int tt_bom32 = 0;
int tt_den32 = 0;
int tt_bom8266 = 0;
int is_auto = 1;

//==================
// SETUP Interrupt : ngắt phát hiện nút nhấn
//==================

void IRAM_ATTR xuly_led32() {
  if (is_auto)
    return;
  tt_den32 = !tt_den32;
  Serial.println("led32:" + tt_den32);
}

void IRAM_ATTR xuly_bom32() {
  if (is_auto)
    return;
  tt_bom32 = !tt_bom32;
  Serial.println("bom32:" + tt_bom32);
}

void IRAM_ATTR xuly_bom8266() {
  if (is_auto)
    return;
  tt_bom8266 = !tt_bom8266;
  Serial.println("tt_bom8266:" + tt_bom8266);
}

void IRAM_ATTR xuly_channel() {
  is_auto = !is_auto;
  delay(50);
  digitalWrite(LED_CHANNEL_PIN, is_auto);
}

void setupInterrupt() {
  pinMode(BTN_LED32_PIN, INPUT);
  pinMode(BTN_PUM32_PIN, INPUT);
  pinMode(BTN_PUM8266_PIN, INPUT);
  pinMode(BTN_CHANNEL_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(BTN_LED32_PIN), xuly_led32, RISING);
  attachInterrupt(digitalPinToInterrupt(BTN_PUM32_PIN), xuly_bom32, RISING);
  attachInterrupt(digitalPinToInterrupt(BTN_PUM8266_PIN), xuly_bom8266, RISING);
  attachInterrupt(digitalPinToInterrupt(BTN_CHANNEL_PIN), xuly_channel, RISING);
  Serial.println("setup Interrupt done!");
}

//===========
// MQTT
//===========
void xuLyLenhTuNodeRed(String cmd) {
  if (cmd == "auto") {
    is_auto = 1;
    digitalWrite(LED_CHANNEL_PIN, HIGH);
    return;
  }

  // xử lý lênh khi yêu cầu thay đổi chế độ thành thủ công(manual)
  if (cmd == "manual") {
    is_auto = 0;
    digitalWrite(LED_CHANNEL_PIN, LOW);
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

  bool led_channel = digitalRead(LED_CHANNEL_PIN);
  String dataGuiNodeRed = "{";

  dataGuiNodeRed += "\"dataManual\":";
  dataGuiNodeRed += String(1);
  dataGuiNodeRed += ",";

  dataGuiNodeRed += "\"led\":";
  dataGuiNodeRed += String(led_channel);
  dataGuiNodeRed += ",";

  dataGuiNodeRed += "\"bom32\":";
  dataGuiNodeRed += String(tt_bom32);
  dataGuiNodeRed += ",";

  dataGuiNodeRed += "\"den32\":";
  dataGuiNodeRed += String(tt_den32);
  dataGuiNodeRed += ",";

  dataGuiNodeRed += "\"bom8266\":";
  dataGuiNodeRed += String(tt_bom8266);
  dataGuiNodeRed += ",";

  dataGuiNodeRed += "\"channel\":";
  dataGuiNodeRed += String(is_auto);
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
  pinMode(NOTI_LED_PIN, OUTPUT);
  pinMode(LED_CHANNEL_PIN, OUTPUT);
  digitalWrite(NOTI_LED_PIN, LOW);
  digitalWrite(LED_CHANNEL_PIN, LOW);

  setupWifi();
  delay(50);

  setupMQTT();
  delay(50);

  setupInterrupt();
  delay(50);
}

void loop() {
  loopSendReport();
  mqtt_client.loop();  // hàm duy trì sự kiện lắng nghe lệnh từ Node-red
}