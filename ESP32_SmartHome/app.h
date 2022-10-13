#ifndef __APP_H__
#define __APP_H__

//library DHT
#include <Adafruit_Sensor.h>
#include <DHT.h>

//============= Var ============
float hum = 0;
float temp = 0;
float buffer_hum = 0;
float buffer_temp = 0;
bool buffer_red_led_status = false;
bool buffer_green_led_status = false;
bool is_press = false;
int count_press = 0;
unsigned long long old_time_press = 0;
unsigned long long delay_time_press =200;
unsigned long long old_time_report = 0;

//==============================

void IRAM_ATTR callbackInterrupt();
void executeMqttCommand(String command);
DHT dht(DHT_PIN, DHTTYPE);

//============= DHT ============
void setupDHT()
{
    dht.begin();
    Serial.println("setup DHT done !!!");
}

void stateLed()
{
    buffer_red_led_status = digitalRead(RED_LED_PIN);
    buffer_green_led_status = digitalRead(GREEN_LED_PIN);
}
void readDHT()
{
    hum = dht.readHumidity();
    temp = dht.readTemperature();
    if (isnan(hum) || isnan(temp)) {
        Serial.println(F("Failed to read from DHT sensor!"));
        buffer_hum = -1;
        buffer_temp = -1;
        return;
    }
    buffer_hum = hum ;
    buffer_temp = temp ;
}

// ============= inout & output ============
void setupInOutput()
{
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(DHT_PIN, INPUT);
    pinMode(INTERRUPT_PIN, INPUT);
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
    //interrupt
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), callbackInterrupt, RISING);
}

//===== logic control led // interrupt ======
void IRAM_ATTR callbackInterrupt()
{
    is_press = true;
    count_press++;
    old_time_press = millis();
}

void handledButton()
{
  if(!is_press)
      return;

  if(millis() - old_time_press > delay_time_press)
  {
      if(count_press == 1)
      {
        buffer_red_led_status = digitalRead(RED_LED_PIN);
        digitalWrite(RED_LED_PIN, !buffer_red_led_status);
        is_press = false;
        count_press = 0;
        Serial.println("red led: " + String(buffer_red_led_status) );
      }
      if(count_press == 2)
      {
        buffer_green_led_status = digitalRead(GREEN_LED_PIN);
        digitalWrite(GREEN_LED_PIN, !buffer_green_led_status);
        is_press = false;
        count_press = 0;
        Serial.println("red led: " + String(buffer_green_led_status) );
      }
      if(count_press > 2)
      {
        Serial.println("count_press > 2");
        is_press = false;
        count_press = 0;
      }
  }
}

//======== report reading ==========
void reportReadding()
{
    // char chip_name[24];
    // sprintf(chip_name, OTA_HOSTNAME, getChipId());

    String json_string = "";

    json_string += "{";

    json_string += "\"chipID\":\"";
    // json_string += String(chip_name);
    json_string += "\",";

    json_string += "\"red_led\":";
    json_string += String(buffer_red_led_status);
    json_string += ",";

    json_string += "\"green_led\":";
    json_string += String(buffer_green_led_status);
    json_string += ",";

    json_string += "\"hum\":";
    json_string += String(buffer_hum);
    json_string += ",";

    json_string += "\"temp\":\"";
    json_string += String(buffer_temp);
    json_string += "\",";

    json_string += "\"now\":\"";
    json_string += String(millis());
    json_string += "\"";

    json_string += "}";

    Serial.println( String(json_string) );
    mqttSendServer(json_string);
    json_string = "";
}



void setupApp()
{
    setupInOutput();
    delay(50);

    //DHT
    setupDHT();
    delay(50);
}


void loopApp()
{   
    //DHT
    readDHT();
    stateLed();

    //led
    handledButton();

    if(millis() - old_time_report > 5000)
    {
        old_time_report = millis();
        reportReadding();
    }    
    
}

#endif