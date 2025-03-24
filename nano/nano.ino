/**
 * Wiegand access control (nano)
 * 
 * 2 door authentication project using Wiegand Keypad/Card reader and 
 * ATMega328P Arduino Nano microcontroller
 * 
 */

void setup() {
  Door1_init();
}

void loop() {
  Door1_loop();
}
