#include <jled.h>


// For some reason, making the setup calls here before elaboration completes
// causes the this pointer in the JLed constructor to be wrong
// I didn't track that down, but the answer is to make the calls post main.
JLed led = JLed(11); //.Breathe(2000).DelayAfter(1000).Forever();
JLed led1 = JLed(13); //.Blink(500,500).Forever();






void setup() { 
   led.Breathe(2000).DelayAfter(1000).Forever();
   led1.Blink(500,500).Forever();


}



void loop() {
  

  JLed::UpdateAll();
} 
