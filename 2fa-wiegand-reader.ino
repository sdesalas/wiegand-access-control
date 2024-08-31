#include <Arduino.h>
#include <FS.h>

void setup() {
  Board_init();
  GPIO_init();
}

void loop() {
  // Do nothing here.
  // Its all handled with timers and interrupts.
}
