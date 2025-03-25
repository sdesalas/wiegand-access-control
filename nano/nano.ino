/**
 * Wiegand access control (nano)
 * 
 * 2 door authentication project using Wiegand Keypad/Card reader and 
 * ATMega328P Arduino Nano microcontroller
 * 
 */
#include "Storage.h"

void setup() {
  Serial.begin(9600);
  Serial.println();
  delay(500);
  Storage::init();
  Door1_init();
}

void loop() {
  Door1_loop();
}
