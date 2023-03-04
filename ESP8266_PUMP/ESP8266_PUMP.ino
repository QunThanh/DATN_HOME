#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>


#define IN1_PIN               13        // D7
#define IN2_PIN               12        // D6
#define NOTI_LED_PIN          2         


WiFiClient client;
PubSubClient mqtt_client(client);

//============
const char* ssid = "NTGD";
const char* pass = "112233445566";
const char *mqttserver = "192.168.1.15"; // ip laptop
const int mqttport = 1883;
const char *mqttid = "pump";
const char *toppicsub = "S-ESP-PUMP";
const char *toppicpub = "P-ESP-PUMP";

unsigned long old_time_report = millis();
unsigned long delay_time_report = 5;       // 5s 

String buffer_data_tu_nodered = "";
int buffer_pump = 00;

//===========
// MQTT
//===========
void xuLyLenhTuNodeRed(String cmd)
{
  // pump
  if (cmd == "open")
  {
    digitalWrite(IN1_PIN, HIGH);
    digitalWrite(IN2_PIN, LOW);
    buffer_pump = 10;
    Serial.println("open pump");      // debug
    return;
  }

  if (cmd == "close")
  {
    digitalWrite(IN1_PIN, LOW);
    digitalWrite(IN2_PIN, LOW);
    buffer_pump = 0;
    Serial.println("close pump");      // debug
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
  Serial.print("cmd(Server):");
  Serial.print(buffer_data_tu_nodered);
  Serial.println(".");
  xuLyLenhTuNodeRed(buffer_data_tu_nodered);
  
  // xóa dữ liệu vừa sử lý xong để 
  // tránh việc dữ liệu trước bị đè vào dữ liệu sau
  buffer_data_tu_nodered = "";
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


// reportMQTT
void reportReadings()
{
    String dataGuiNodeRed = "{";

    // last ID login 
    dataGuiNodeRed += "\"pump\":";
    dataGuiNodeRed += String(buffer_pump);
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
    if (!success)
    {
        Serial.println("Failed to publish to MQTT");
        return;
    }
}


void setupMQTT()
{
  Serial.println("Connecting MQTT");
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
  Serial.println("Connecting WiFi");
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
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, LOW);
  digitalWrite(NOTI_LED_PIN, LOW);

  setupWifi();              
  delay(50);

  setupMQTT();
  delay(50);
}

void loop() {
  loopSendReport();
  mqtt_client.loop();       // hàm duy trì sự kiện lắng nghe lệnh từ Node-red
}
