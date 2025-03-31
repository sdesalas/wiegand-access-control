/**
 * Adds factory reset functionality.
 * 
 * Momentary PUSH button connecting to ground. Hold to trigger.
 * 
 * If the button is held down for 5 seconds we get a factory reset.
 */
#include "Constants.h"
#include "lib/AsyncTask.h"
#include "PinChangeInterrupt.h"

#define RESET_DETECTION_PIN 12 // D12 -> Connect via PUSH button to GROUND
#define RESET_PUSH_DURATION (5*SECOND)

AsyncTask resetTask;

void Reset_checkFactoryReset() {
  Serial.println("WARNING! FACTORY RESET pin pressed.");
  Serial.println("Checking in 5 seconds..");
  resetTask.once([]() {
    if (digitalRead(RESET_DETECTION_PIN) == LOW) { // LOW = pressed (INPUT_PULLUP)
      Serial.println("Factory reset detected!");
      Storage::factoryReset();
      delay(200*MS);
      reboot();
    } else {
      Serial.println("False alarm. Skipping..");
    }
  }, RESET_PUSH_DURATION);
}

void Reset_init() {
  // Add factory reset
  pinMode(RESET_DETECTION_PIN, INPUT_PULLUP);
  attachPCINT(digitalPinToPCINT(RESET_DETECTION_PIN), Reset_checkFactoryReset, FALLING);
}

void Reset_loop() {
  resetTask.loop();
}
