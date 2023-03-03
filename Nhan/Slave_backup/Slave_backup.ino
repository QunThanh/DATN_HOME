#include <DHT.h>

#define led               13
#define pump              14

#define btn_led           12
#define btn_pump          27
#define btn_channel       26

#define DHT_PIN           4
#define cambien_anhsang   25
#define cambien_doam_dat  33
#define cambien_mua       32


String buffer_cambien_anhsang = "0";    // dữ liệu tạm thời lấy từ cảm biến ánh sáng
String buffer_cambien_mua = "0";        // dữ liệu tạm thời lấy từ cảm biến mưa
String buffer_cambien_doam_dat = "0";   // dữ liệu tạm thời lấy từ cảm biến độ ẩm đất
String buffer_nhietdo = "0";            // dữ liệu tạm thời lấy từ cảm biến nhiệt độ DHT11
String buffer_doam = "0";               // dữ liệu tạm thời lấy từ cảm biến độ ẩm DHT11
float doam = 0;
float nhietdo = 0;

String buffer_led = "0";                // dữ liệu tạm thời trạng thái của led
String buffer_pump = "0";               // dữ liệu tạm thời trạng thái của máy bơm
String buffer_channel = "0";            // dữ liệu tạm thời trạng thái của chế độ

String temp_data = "";                  // dữ liệu tạm thời nhận từ UART
String command = "";                    // lệnh từ Master gửi xuống

int trangthai_led = 0;
int trangthai_pump = 0;
int threshold = 28;

bool is_auto = true;
bool interrupt_busy = false;

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
    if (isnan(doam) || isnan(nhietdo))
      {
        buffer_nhietdo = "00.0";
        buffer_doam = "00.0";
        return;
      }
    
    buffer_nhietdo = String(nhietdo);
    buffer_doam = String(doam);
}

//==================
// SETUP Interrupt
//==================
void IRAM_ATTR xuly_btn_led()
{
  if (is_auto)
    return;

  if (interrupt_busy)
    return;

  interrupt_busy = true;
  trangthai_led = digitalRead(led);           // đọc trạng thái của led hiện tại.
  digitalWrite(led, !trangthai_led);          // đảo trạng thái led
  buffer_led = String(!trangthai_led);        // đưa vào biến tạm để tý nữa gửi lên master
  interrupt_busy = false;
  return;
}

void IRAM_ATTR xuly_btn_pump()
{
  if (is_auto)
    return;

  if (interrupt_busy)
    return;
  
  interrupt_busy = true;
  trangthai_pump = digitalRead(pump);         // đọc trạng thái của máy bơm hiện tại.
  digitalWrite(pump, !trangthai_pump);        // đảo trạng thái máy bơm
  buffer_pump = String(!trangthai_pump);      // đưa vào biến tạm để tý nữa gửi lên master
  interrupt_busy = false;
  return;
}
void IRAM_ATTR xuly_btn_channel()
{
  if (is_auto)
    return;

  if (interrupt_busy)
    return;
  
  interrupt_busy = true;
  is_auto = !is_auto;                         // đảo trạng thái channel
  buffer_channel = String(is_auto);           // đưa vào biến tạm để tý nữa gửi lên master
  interrupt_busy = false;
  return;
}

void setupInterrupt()
{
  pinMode(btn_channel, INPUT);
  pinMode(btn_pump, INPUT);
  pinMode(btn_led, INPUT);
  attachInterrupt(digitalPinToInterrupt(btn_channel), xuly_btn_channel, RISING);
  attachInterrupt(digitalPinToInterrupt(btn_pump), xuly_btn_pump, RISING);
  attachInterrupt(digitalPinToInterrupt(btn_led), xuly_btn_led, RISING);
  Serial.println("setup Interrupt done!");
}

//==================
// xử lý tự động
//==================
void xuLyTuDong()
{
  if (!is_auto)
    return;

  if (nhietdo > threshold)
  {
    digitalWrite(pump,HIGH);
    buffer_pump = "1";
  }
  else 
  {
    digitalWrite(pump,LOW);
    buffer_pump = "0";
  }

  if (cambien_anhsang == 0) 
  {
    digitalWrite(led,LOW);
    buffer_led = "0";
  }
  else 
  {
    digitalWrite(led,HIGH);
    buffer_led = "1";
  }

}

//====================================
// xu ly lenh tu master
//====================================
void xuLyLenhMaster(String cmd)
{
  int num_index = cmd.indexOf(";");
  if(num_index > 0)
  {
    String str_threshold = cmd.substring(num_index);
    int int_threshold = str_threshold.toInt();
    threshold = int_threshold;
    return;
  }

  if (cmd == "auto")
  {
    is_auto = true;
    return;
  }
  if (cmd == "manual")
  {
    is_auto = false;
    return;
  }

  if (is_auto)
    return;

  //control pump from master
  if (cmd == "onpump")
  {
    digitalWrite(pump,HIGH);
    buffer_pump = "1";
    return;
  }

  if (cmd == "offpump")
  {
    digitalWrite(pump,LOW);
    buffer_pump = "0";
    return;
  }

  //control led from master
  if (cmd == "onled")
  {
    digitalWrite(led,HIGH);
    buffer_led = "1";
    return;
  }

  if (cmd == "offled")
  {
    digitalWrite(led,LOW);
    buffer_led = "0";
    return;
  }
}

//====================================
// Nhận tin từ Master gửi bằng UART
//====================================
void docDataTuMaster()
{
  while (Serial2.available())
  {
    char incomingString = char(Serial2.read()); // hàm đọc từng byte
    if (incomingString != '*')
    {
      temp_data += incomingString;
    }
    else
    {
      command = temp_data;              
      Serial.println(command);
      temp_data = "";                         // reset du~ lieu

      xuLyLenhMaster(command);                // sử lý lệnh từ master
      command = "";                           // reset du~ lieu
    }
  }
}

//====================================
// tong hop du lieu 
// gui data toi Master
//====================================
void layGiaTriThietBi()
{
  buffer_cambien_doam_dat =  String(digitalRead(cambien_doam_dat));
  buffer_cambien_mua = String(digitalRead(cambien_mua));
  buffer_cambien_anhsang = String(digitalRead(cambien_anhsang));
  buffer_pump = String(digitalRead(pump));
  buffer_led = String(digitalRead(led));
  buffer_channel = String(is_auto);
}

void guiDataToiMaster()
{
  String data_tam_thoi = "";
  
  data_tam_thoi = buffer_nhietdo;
  data_tam_thoi +=  "I" + buffer_doam;
  data_tam_thoi +=  "I" + buffer_cambien_anhsang;
  data_tam_thoi +=  "I" + buffer_cambien_mua;
  data_tam_thoi +=  "I" + buffer_cambien_doam_dat;
  data_tam_thoi +=  "I" + buffer_pump;
  data_tam_thoi +=  "I" + buffer_led;
  data_tam_thoi +=  "I" + buffer_channel;

  data_tam_thoi +=  "*";
  Serial.println("data_gui_master:" + data_tam_thoi);     // in ra serial de? debug
  Serial2.println(data_tam_thoi);                         // gui data toi Master

  data_tam_thoi = "";
}

// run
void setup()
{
  Serial.begin(9600);
  Serial2.begin(9600);
  pinMode(pump, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(cambien_anhsang, INPUT);
  pinMode(cambien_doam_dat, INPUT);
  pinMode(cambien_mua, INPUT);
  digitalWrite(pump, LOW);
  digitalWrite(led, LOW);

  setupInterrupt();
  delay(50);

  setupDHT();
  delay(50);
}

void loop()
{
  xuLyTuDong();

  layGiaTriTuDHT();
  layGiaTriThietBi();

  guiDataToiMaster();
  delay(100);
  
  docDataTuMaster();
  delay(100);         //phải có delay ở đây để có thể đọc kịp dữ liệu từ ESP-Master
}
