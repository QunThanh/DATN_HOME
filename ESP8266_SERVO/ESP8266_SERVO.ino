#include <Servo.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

#define LCD_I2C_ADDR          0x27     
#define LCD_I2C_NUM_COL       16   
#define LCD_I2C_NUM_ROW       2  
// SDA 4 (D2)
// SCL 5 (D1)
#include "LCD.h"

#define SERVO_PIN             15        // D8

// SS/SDA     2  (D4)
// SCK        14 (D5)
// MOSI/SCL   13 (D7)
// MISO       12 (D6)
// RST        0  (D3)
#define RST_PIN               0         // D3
#define SS_PIN                2         // D4


WiFiClient client;
PubSubClient mqtt_client(client);
MFRC522 rfid(SS_PIN, RST_PIN);
Servo servo;

//============
const char* ssid = "NTGD";
const char* pass = "112233445566";
const char *mqttserver = "192.168.1.7"; // ip laptop
const int mqttport = 1883;
const char *mqttid = "gate";
const char *toppicsub = "S-ESP-GATE";
const char *toppicpub = "P-ESP-GATE";

unsigned long old_time_report = millis();
unsigned long last_noti_time = millis();
unsigned long delay_time_report = 5;       // 5s 

String buffer_data_tu_nodered = "";
byte bufferID[4];
int notification = 0;           // 1 : mean open, 0 : do nothing, -1 : wrong ID
bool is_open_door = false;
uint8_t threshold_open = 200;


//===========
// MQTT
//===========
void xuLyLenhTuNodeRed(String cmd)
{
  // threshold door open
  // range 0-255
  int num_index = cmd.indexOf(";");
  if(num_index > 0)
  {
    String str_threshold = cmd.substring(num_index + 1);  
    int int_threshold = str_threshold.toInt();
    if (int_threshold < 0 || int_threshold > 255)
    {
      Serial.println("threshold over range");
      return;
    }

    threshold_open = int_threshold;  
    Serial.println("threshold" + str_threshold + ".");             // debug
    return;
  }

  // Door
  if (cmd == "open")
  {
    notification = 1;
    servo.write(threshold_open);
    Serial.println("open the door");      // debug
    return;
  }

  if (cmd == "close")
  {
    notification = 0;
    servo.write(0);
    Serial.println("close the door");      // debug
    return;
  }

  if (cmd == "wrong")
  {
    notification = -1;
    Serial.println("wrong ID");      // debug
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
  Serial.print("cmd Node-red (8266): ");
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
  Serial.println("connected:" + String(mqtt_client.connected()) + ".");
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
    dataGuiNodeRed += "\"lastID\":\"";
    dataGuiNodeRed += String((char*)bufferID);
    dataGuiNodeRed += "\",";

    // notification
    dataGuiNodeRed += "\"noti\":";
    dataGuiNodeRed += String(notification);
    dataGuiNodeRed += ",";

    // status door
    dataGuiNodeRed += "\"door\":";
    dataGuiNodeRed += String(is_open_door);
    dataGuiNodeRed += ",";

    // servo deg
    dataGuiNodeRed += "\"deg\":";
    dataGuiNodeRed += String(servo.read());
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

//==========
//servo
//==========
void setupServo() {
  servo.attach(SERVO_PIN); 
  delay(10);
  servo.write(100);
  delay(100);
  servo.write(0);
  Serial.println("setup servo done!");
}


///==========
// RFDI
///==========
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void setupRFID() {
  SPI.begin();		
	rfid.PCD_Init();		
	rfid.PCD_DumpVersionToSerial();

	Serial.println("setup RFID done");
}

void loopRFID() {
  	// Look for new cards
	if ( ! rfid.PICC_IsNewCardPresent()) {
		return;
	}
 
	// Select one of the cards
	if ( ! rfid.PICC_ReadCardSerial()) {
		return;
	}
  
  if (rfid.uid.uidByte[0] != bufferID[0] || 
    rfid.uid.uidByte[1] != bufferID[1] || 
    rfid.uid.uidByte[2] != bufferID[2] || 
    rfid.uid.uidByte[3] != bufferID[3] ) {
    Serial.println(F("A new card has been detected."));

    // Store NUID into bufferID array
    for (byte i = 0; i < 4; i++) {
      bufferID[i] = rfid.uid.uidByte[i];
    }
   
    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
  }
  else Serial.println(F("Card read previously."));
	// Dump debug info about the card; PICC_HaltA() is automatically called

  rfid.PICC_HaltA();
  
  rfid.PCD_StopCrypto1();

  guiDataLenNodered(String((char*)bufferID));
  delay(50);
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

void loopLCD(){
  if(notification == 0)
  {
    //colum, row, message
    clearLcd();
    printLcd(0,0,"Xin nhap the");
    printLcd(0,1,"ben nay ---->");
    delay(100);
    return;
  }
  
  if(notification == 1)
  {
    clearLcd();
    printLcd(0,0,"The chinh xac.");
    printLcd(0,1,"----> Moi vao");
    delay(100);
    return;
  }
  
  clearLcd();
  printLcd(0,0,"The k chinh xac.");
  printLcd(0,1,"Xin quet lai the");
  delay(100);
}


void setup() {
  Serial.begin(115200);
  // pinMode(LED_PIN,OUTPUT);
  // digitalWrite(LED_PIN, LOW);

  setupWifi();              
  delay(50);

  setupMQTT();
  delay(50);

  setupServo();
  delay(50);

  setupRFID();
  delay(50);

  setupLcd();
  delay(50);
}

void loop() {

  loopRFID();

  mqtt_client.loop();       // hàm duy trì sự kiện lắng nghe lệnh từ Node-red
  
  loopSendReport();

  loopLCD();
}
