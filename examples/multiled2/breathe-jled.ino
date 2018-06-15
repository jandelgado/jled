#include <jled.h>


JLed led = JLed(11).Breathe(2000).DelayAfter(1000).Forever();
JLed led1 = JLed(13).Blink(500,500).Forever();






void setup() { 
}



void loop() {
  JLed::UpdateAll();
} 
