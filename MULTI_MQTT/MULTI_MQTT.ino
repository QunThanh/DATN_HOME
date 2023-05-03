// #include <Servo.h>
// #include <ESP8266WiFi.h>
#include <WiFi.h>
#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

#define LCD_I2C_ADDR 0x27
#define LCD_I2C_NUM_COL 16
#define LCD_I2C_NUM_ROW 2
// SDA 4 (D2)
// SCL 5 (D1)
#include "my_lcd.h"

// #define SERVO_PIN             2        // D4

// SS/SDA     2  (D4)
// SCK        14 (D5)
// MOSI/SCL   13 (D7)
// MISO       12 (D6)
// RST        0  (D3)
#define RST_PIN 0  // D3
#define SS_PIN 16  // D0


WiFiClient client;
PubSubClient mqtt_client(client);

WiFiClient client2;
PubSubClient mqtt_client2(client2);

//============
const char *ssid = "Sophie";
const char *pass = "aaaaaaaaaa";
const char *mqttserver = "192.168.1.150";  
const int mqttport = 1883;
const char *mqttid = "gate";
const char *toppicsub = "S-ESP-GATE";
const char *toppicpub = "P-ESP-GATE";
const char *toppicpubID = "P-ID-ESP-GATE";

const char *mqttserver2 = "128.199.64.166"; 
const int mqttport2 = 1884;
#define MQTT_USERNAME                 "originallyus"
#define MQTT_PASSWORD                 "originallyus"
const char *mqttid2 = "gate";
const char *toppicsub2 = "S-ESP-GATE";
const char *toppicpub2 = "P-ESP-GATE";
const char *toppicpubID2 = "P-ID-ESP-GATE";

unsigned long old_time_report = millis();
unsigned long delay_time_report = 5;  // 5s

String buffer_data_tu_nodered = "";
String buffer_id = "";
String strID = "";
int notification = 0;      // 1 : mean open, 0 : do nothing, -1 : wrong ID
int threshold_open = 199;  // if value is < 200



//===========
// MQTT
//===========
void xuLyLenhTuNodeRed(String cmd) {
  // threshold door open
  // range 0-199
  int num_index = cmd.indexOf(";");
  if (num_index > 0) {
    String str_threshold = cmd.substring(num_index + 1);
    int int_threshold = str_threshold.toInt();
    if (int_threshold < 0 || int_threshold > 255) {
      Serial.println("threshold over range");
      return;
    }

    threshold_open = int_threshold;
    Serial.println("threshold" + str_threshold + ".");  // debug
    return;
  }

  // Door
  if (cmd == "open") {
    notification = 1;
    // servo.write(threshold_open);
    delay(500);
    Serial.println("open the door");  // debug
    return;
  }

  if (cmd == "close") {
    notification = 0;
    // servo.write(0);
    delay(500);
    Serial.println("close the door");  // debug
    return;
  }

  if (cmd == "wrong") {
    notification = -1;
    Serial.println("wrong ID");  // debug
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
  Serial.print("cmd Node-red (8266):");
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

bool guiDataIDLenNodered(String data_gui_nodered) {
  if (mqtt_client.connected()) {
    mqtt_client.publish(toppicpubID, data_gui_nodered.c_str());
    return true;
  }
  return false;
}

// reportMQTT
void reportReadings() {
  String dataGuiNodeRed = "{";

  // last ID login
  dataGuiNodeRed += "\"lastID\":\"";
  dataGuiNodeRed += String(buffer_id);
  dataGuiNodeRed += "\",";

  // notification
  dataGuiNodeRed += "\"noti\":";
  dataGuiNodeRed += String(notification);
  dataGuiNodeRed += ",";

  // servo deg
  dataGuiNodeRed += "\"deg\":";
  // dataGuiNodeRed += String(servo.read());
  dataGuiNodeRed += ",";

  // general info
  dataGuiNodeRed += "\"ip\":\"";
  dataGuiNodeRed += WiFi.localIP().toString();
  dataGuiNodeRed += "\",";

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

void setupMQTT2() {
  Serial.println("Connecting MQTT 2");
  mqtt_client2.setServer(mqttserver2, mqttport2);
  mqtt_client2.setCallback(callback);

  while (!mqtt_client2.connect(mqttid2,"originallyus","originallyus")) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected MQTT2");
  mqtt_client2.publish(toppicpub2, "Đã kết nối và chờ lệnh");
  mqtt_client2.subscribe(toppicsub2);
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
    reportReadings();
    old_time_report = millis();
    delay(50);
    return;
  }
  delay(50);
}


// void setup(){

// }
// void loop(){

//}

//===============================================


void loop1() {
  if(!mqtt_client.connected())
  {
    setupMQTT();
    return;
  }

  loopSendReport();

  mqtt_client.loop();  // hàm duy trì sự kiện lắng nghe lệnh từ Node-red
  if(!mqtt_client2.connected()) delay(5000);
  else delay(50);
}


void loop2() {
  if(!mqtt_client2.connected())
  {
    setupMQTT2();
    return;
  }

  loopSendReport();

  mqtt_client2.loop();
  if(!mqtt_client.connected()) delay(5000);
  else delay(50);
}

//Task1code: blinks an LED every 1000 ms
void Task1code(void *pvParameters) {
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {
    loop1();
  }
}

//Task2code: blinks an LED every 700 ms
void Task2code(void *pvParameters) {
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {
    loop2();
  }
}

TaskHandle_t Task1;
TaskHandle_t Task2;

void setup() {
  Serial.begin(115200);

  setupWifi();

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
    Task1code, /* Task function. */
    "Task1",   /* name of task. */
    10000,     /* Stack size of task */
    NULL,      /* parameter of the task */
    1,         /* priority of the task */
    &Task1,    /* Task handle to keep track of created task */
    0);        /* pin task to core 0 */
  delay(500);

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
    Task2code, /* Task function. */
    "Task2",   /* name of task. */
    10000,     /* Stack size of task */
    NULL,      /* parameter of the task */
    1,         /* priority of the task */
    &Task2,    /* Task handle to keep track of created task */
    1);        /* pin task to core 1 */
  delay(500);
}

void loop(){
  delay(10);
};


//==============================================

