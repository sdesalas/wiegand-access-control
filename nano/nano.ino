/**
 * Wiegand access control (nano)
 * 
 * 2 door authentication project using Wiegand Keypad/Card reader and 
 * ATMega328P Arduino Nano microcontroller
 * 
 */
#include "Storage.h"

void(* reboot) (void) = 0; //declare reset function @ address 0

void setup() {
  Serial.begin(9600);
  Serial.println();
  delay(500);
  Storage::init();
  Door1_init();
  Door2_init();
  Reset_init();
}

void loop() {
  Door1_loop();
  Door2_loop();
  Reset_loop();
}
