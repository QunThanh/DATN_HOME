#include <Arduino.h>
#include <IRsend.h>
#include <IRrecv.h>

const uint16_t SEND_PIN = 4;    // D2
const uint16_t Led = 12;        // D6
const uint16_t RECV_PIN = 5;    // D1

IRsend irSend(SEND_PIN);  
IRrecv irRecv(RECV_PIN); 

int min_repeat = 2;
uint8_t buffer_type = -1;
uint32_t buffer_value = 0xff;
uint16_t buffer_bits = 0;
uint32_t buffer_len = 0;

decode_results results;

uint16_t rawData[24] = {23,1150,500,1150,550,300,1350,1150,550,1100,550,300,1350,300,1400,300,1350,1150,550,300,1350,300,1350,350};

//=====setup======
void setupIrSend(){
  irSend.begin();
}

void setupIrRecv(){
  irRecv.enableIRIn();
}

//=====func send======
void sendRawIRCodeWithRawArray(unsigned int *rawCodesArray){
  int len = 32;
  
  irSend.sendRaw(rawData, 24, 38);    // 38 is 38kHz 
  delay(10);                          // Wait a bit between retransmissions
  irRecv.enableIRIn();                // re-start Recv
}

void sendIRCode(int codeType, unsigned long codeValue, int codeLength, int repeat )
{
  if (codeType == NEC && repeat)
  {
    Serial.print("NEC & Repeat");
    irSend.sendNEC(repeat, codeLength);
    irRecv.enableIRIn();
    return;
  }
  
  if (codeType == NEC && !repeat)
  {
    Serial.print("NEC");
    Serial.println(codeValue, HEX);
    irSend.sendNEC(codeValue, codeLength);
    irRecv.enableIRIn();
    return;
  }
  
  if (codeType == SONY)
  {
    Serial.print("SONY");
    Serial.println(codeValue, HEX);
    irSend.sendSony(codeValue, codeLength);
    irRecv.enableIRIn();
    return;
  } 

  Serial.println("Error: should send with func sendRawIRCodeWithRawArray()");
  irRecv.enableIRIn();
  return;
}

//=====func Recv======
void decodeIr() {

  if ( !irRecv.decode(&results) ) return;

  buffer_type = results.decode_type;
  buffer_value = results.value;
  buffer_bits = results.bits;
  buffer_len = results.rawlen;
  Serial.println("===H===");
  
  if (buffer_type == NEC)
  {
    Serial.print("NEC:");
    Serial.println(buffer_value, HEX);
    Serial.println("bits:" + String(buffer_bits));
    Serial.println("len:" + String(buffer_len));
    return;
  }
  
  if (buffer_type == SONY) 
  {
    Serial.print("SONY:");
    Serial.println(buffer_value, HEX);
    Serial.println("bits:" + String(buffer_bits));
    Serial.println("len:" + String(buffer_len));
    return;
  }
  
  Serial.print("UNKNOWN:");
  Serial.println("type:" + String(buffer_type));
  Serial.println(buffer_value, HEX);
  Serial.println("bits:" + String(buffer_bits));
  Serial.println("len:" + String(buffer_len));
  return;
}

//=====run======
void setup() {
  Serial.begin(115200);
  
  setupIrSend();
  setupIrRecv();
  
  pinMode(Led, OUTPUT);
  digitalWrite(Led, LOW);
}


void loop() {
  if (!irRecv.decode(&results)) return;

  digitalWrite(Led, HIGH);
  decodeIr();
  digitalWrite(Led, LOW);
  irRecv.resume();
  delay(500);

  //test
  if (buffer_value == 0xA90)    sendIRCode(SONY, 0xA90, 12, 2);
    
  delay(50);
}
