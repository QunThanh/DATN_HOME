/*
 * IRremoteESP8266: IRrecvDemo - demonstrates receiving IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Version 0.1 Sept, 2015
 * Based on Ken Shirriff's IrsendDemo Version 0.1 July, 2009, Copyright 2009 Ken Shirriff, http://arcfn.com
 */
#include <Arduino.h>
#include <IRrecv.h>

int RECV_PIN = 19; //an IR detector/demodulatord is connected to GPIO pin D6

IRrecv irrecv(RECV_PIN);

decode_results results;
int type = -1;

void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
}

void loop() {

  
  if (irrecv.decode(&results)) {
    type = results.decode_type;
     Serial.println("==H==");
      if (type == NEC)
     {
      Serial.println("NEC");
      Serial.println("type:" + String(results.decode_type));
     }
     else if (type == SONY) 
     {
      Serial.println("SONY");
      Serial.println("type:" + String(results.decode_type));
     }
     else
     {
      Serial.println("UNKNOWN");
      Serial.println("type:" + String(results.decode_type));
     }
     
     Serial.println(results.value, HEX);
     Serial.println("bits:" + String(results.bits));
     Serial.println("len:" + String(results.rawlen));
    irrecv.resume(); // Receive the next value
  }
  delay(100);
}
