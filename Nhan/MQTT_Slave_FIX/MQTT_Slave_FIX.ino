#include <DHT.h>

#define led               13
#define led_channel       23

#define btn_led           12
#define btn_pump          27
#define btn_channel       26

#define DHT_PIN           4
#define cambien_anhsang   25
#define cambien_doam_dat  33
#define cambien_mua       32


String buffer_cambien_anhsang = "0";    // dữ liệu tạm thời lấy từ cảm biến ánh sáng 0 sáng 1 tắt
String buffer_cambien_mua = "0";        // dữ liệu tạm thời lấy từ cảm biến mưa
String buffer_cambien_doam_dat = "0";   // dữ liệu tạm thời lấy từ cảm biến độ ẩm đất
String buffer_nhietdo = "0";            // dữ liệu tạm thời lấy từ cảm biến nhiệt độ DHT11
String buffer_doam = "0";               // dữ liệu tạm thời lấy từ cảm biến độ ẩm DHT11
String buffer_threshold = "0";          // dữ liệu tạm thời lấy từ biến threshold
float doam = 0;
float nhietdo = 0;

String buffer_led = "0";                // dữ liệu tạm thời trạng thái của led
String buffer_channel = "0";            // dữ liệu tạm thời trạng thái của chế độ

String temp_data = "";                  // dữ liệu tạm thời nhận từ UART
String command = "";                    // lệnh từ Master gửi xuống

int trangthai_led = 0;
int trangthai_pump = 0;
int threshold = 3000;                   //0-4020

bool is_press_btn_led = false;
bool is_press_btn_channel = false;
bool is_auto = true;

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
        buffer_nhietdo = "00.0";
        buffer_doam = "00.0";
        return;
      }
      
    buffer_nhietdo = String(nhietdo);
    buffer_doam = String(doam);
}

//==================
// SETUP Interrupt : ngắt phát hiện nút nhấn
//==================
void IRAM_ATTR xuly_btn_led()
{
  if (is_auto)
    return;
  is_press_btn_led = true;
}

void IRAM_ATTR xuly_btn_pump()
{
  if (is_auto)
    return;
  trangthai_pump = !trangthai_pump;       // đọc trạng thái của máy bơm hiện tại.
}
void IRAM_ATTR xuly_btn_channel()
{
  is_press_btn_channel = true;
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
  // nếu khác auto thì không làm hàm này nữa
  if (!is_auto)
    return;

  if (buffer_cambien_doam_dat.toInt() > threshold)  //buffer_cambien_doam_dat trả về là những só càng lớn thì đất càng khô  (mở bơm)
  {
    trangthai_pump = 1;
  }
  else                                             // số bé thì là độ ẩm cao(nhiều nước)   (tắt bơm).
  {
    trangthai_pump = 0;
  }

  if (buffer_cambien_anhsang.toInt() < 1) // trả về 0 thì là trời sáng (nên cần tắt đèn). 
  {
    digitalWrite(led,LOW);
    buffer_led = "0";
  }
  else                     //  = 1 thì là trời tối (cần mở đèn) 
  {
    digitalWrite(led,HIGH);
    buffer_led = "1";
  }
}

//==================
// xu ly nut nhan
//==================
void xuLyNutNhan()
{
  // kiểm tra có nút nào nhấn chưa?  nếu rồi thì xuống dưới thực hiện không thì bỏ qua hàm này
  if (!is_press_btn_led && !is_press_btn_channel)
    return;

  // sử lý việc đổi chế độ hoạt động nếu phát hiện ra là có người nhấn nút đổi chế độ
  if (is_press_btn_channel)
  {
    is_auto = !is_auto;                         // đảo trạng thái channel
    buffer_channel = String(is_auto);           // đưa vào biến tạm để tý nữa gửi lên master
    is_press_btn_channel = false;               // reset trạng thái nút nhấn
    digitalWrite(led_channel, is_auto);
    return;
  }

  // kiểm tra có phải là chế độ tự động không? nếu đúng thì bỏ hàm này. nếu sai thì xuống làm tiếp
  if (is_auto)
    return;

  // sử lý việc đổi trạng thái led nếu phát hiện ra là có người nhấn nút led
  if (is_press_btn_led)
  {
    trangthai_led = digitalRead(led);           // đọc trạng thái của led hiện tại.
    digitalWrite(led, !trangthai_led);          // đảo trạng thái led
    buffer_led = String(!trangthai_led);        // đưa vào biến tạm để tý nữa gửi lên master
    is_press_btn_led = false;                   // reset trạng thái nút nhấn
    return;
  }
}

//====================================
// xu ly lenh tu master
//====================================
void xuLyLenhMaster(String cmd)
{
  // xử lý lênh khi yêu cầu thay đổi ngưỡng
  int num_index = cmd.indexOf(";");
  if(num_index > 0)
  {
    String str_threshold = cmd.substring(num_index + 1);
    int int_threshold = str_threshold.toInt();
    threshold = int_threshold;
    buffer_threshold = str_threshold;
    Serial.println("cmd:" + str_threshold + ".");
    return;
  }

  // xử lý lênh khi yêu cầu thay đổi chế độ thành auto
  if (cmd == "auto")
  {
    is_auto = true;
    buffer_channel = "1";
    digitalWrite(led_channel, HIGH);
    return;
  }

  // xử lý lênh khi yêu cầu thay đổi chế độ thành thủ công(manual) 
  if (cmd == "manual")
  {
    is_auto = false;
    buffer_channel = "0";
    digitalWrite(led_channel, LOW);
    return;
  }

  // kiểm tra có phải là chế độ tự động không? nếu đúng thì bỏ hàm này. nếu sai thì xuống làm tiếp
  if (is_auto)
    return;

  if (cmd == "onpump")
  {
    trangthai_pump = 1;
    return;
  }

  // xử lý lênh khi yêu cầu tắt bơm
  if (cmd == "offpump")
  {
    trangthai_pump = 0;
    return;
  }

   // xử lý lênh khi yêu cầu mở led
  if (cmd == "onled")
  {
    digitalWrite(led,HIGH);
    trangthai_led = digitalRead(led);
    buffer_led = String(trangthai_led);
    return;
  }
  
  // xử lý lênh khi yêu cầu tắt led
  if (cmd == "offled")
  {
    digitalWrite(led,LOW);
    trangthai_led = digitalRead(led);
    buffer_led = String(trangthai_led);
    return;
  }
}

//====================================
// Nhận tin từ Master gửi bằng UART
//====================================
// onled* 
void docDataTuMaster()
{
  while (Serial2.available())
  {
    char incomingString = char(Serial2.read()); // hàm đọc từng byte
    if (incomingString != '*')
    {
      temp_data += incomingString;  // temp_data = onled
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
  buffer_cambien_doam_dat =  String(analogRead(cambien_doam_dat));
  buffer_cambien_mua = String(analogRead(cambien_mua));
  buffer_cambien_anhsang = String(digitalRead(cambien_anhsang));
  buffer_led = String(digitalRead(led));
  buffer_channel = String(is_auto);
  buffer_threshold = String(threshold);
}

void guiDataToiMaster()
{
  String data_tam_thoi = "";
  //tI00.0hI00.0aI1mI4020dI4020bI1lI1cI0
  //t00.0_h00.0_a1_m4020_d4020_b1_l1_c0
  data_tam_thoi =   "tI" + buffer_nhietdo;
  data_tam_thoi +=  "hI" + buffer_doam;
  data_tam_thoi +=  "nI" + buffer_threshold;
  data_tam_thoi +=  "aI" + buffer_cambien_anhsang;
  data_tam_thoi +=  "mI" + buffer_cambien_mua;
  data_tam_thoi +=  "dI" + buffer_cambien_doam_dat;
  if (trangthai_pump) data_tam_thoi +=  "bI10";
  else data_tam_thoi +=  "bI00";
  data_tam_thoi +=  "lI" + buffer_led;
  data_tam_thoi +=  "cI" + buffer_channel;

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
  pinMode(led, OUTPUT);
  pinMode(led_channel, OUTPUT);
  pinMode(cambien_anhsang, INPUT);
  pinMode(cambien_doam_dat, INPUT);
  pinMode(cambien_mua, INPUT);
  digitalWrite(led, LOW);
  digitalWrite(led_channel, LOW);

  setupInterrupt();
  delay(50);

  setupDHT();
  delay(50);
}

void loop()
{
  xuLyNutNhan();
  xuLyTuDong();

  layGiaTriTuDHT();
  layGiaTriThietBi();

  guiDataToiMaster();
  delay(150);
  
  docDataTuMaster();
  delay(150);         //phải có delay ở đây để có thể đọc kịp dữ liệu từ ESP-Master
}