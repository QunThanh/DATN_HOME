#ifndef __MY_INTERRUPT_H__
#define __MY_INTERRUPT_H__

//############ define tag #############
#ifndef NUM_PIN_INTERRUPT
    #define NUM_PIN_INTERRUPT   1
#endif

#ifndef EN_INTERRUPT
    #define EN_INTERRUPT        false
#endif

#ifndef MODE_INTERRUPT
    #define MODE_INTERRUPT      RISING
#endif

//function will be write in app.h
void IRAM_ATTR callbackInterrupt(); 
//#######################################
void defineIOInterrupt()
{
  for (int i = 0; i < NUM_PIN_INTERRUPT; i++)
  {
      pinMode(list_io_interrupt[i], INPUT);
      attachInterrupt(digitalPinToInterrupt(list_io_interrupt[i]), callbackInterrupt, MODE_INTERRUPT);
      Serial.println( "IO" + String(list_io_interrupt[i]) + " has become interruptIO");
      delay(10);
  }
}

//########## setup interrupt ############
void setupInterrupt()
{
  if(!EN_INTERRUPT || NUM_PIN_INTERRUPT <= 0)
  return;
  
  Serial.println("=======INTERRUPT=========");
  defineIOInterrupt();
  Serial.println("Setup interrupt done !");
  Serial.println("=========================");
  
}
//#######################################

#endif