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

namespace Reset {
  AsyncTask task;
  unsigned int taskId;

  void checkFactoryReset() {
    Serial.println("WARNING! FACTORY RESET pin pressed.");
    Serial.println("Checking in 5 seconds..");
    if (taskId > 0) task.remove(taskId);
    taskId = task.once([]() {
      taskId = 0;
      if (digitalRead(RESET_DETECTION_PIN) == LOW) { // LOW = pressed (INPUT_PULLUP)
        Serial.println("Factory reset detected!");
        blink(10);
        Storage::factoryReset();
        delay(200*MS);
        reboot();
      } else {
        Serial.println("False alarm. Skipping..");
      }
    }, RESET_PUSH_DURATION);
  }

  void blink(byte times) {
    blink(times, 500);
  }

  void blink(byte times, unsigned int interval) {
    byte toggle = 0;
    for (byte i = 0; i < times * 2; i++) {
      toggle = toggle ? 0 : 1;
      digitalWrite(LED_BUILTIN, toggle);
      delay(interval / 2);
    }
    digitalWrite(LED_BUILTIN, 0);
  }

  void init() {
    // Add factory reset
    pinMode(RESET_DETECTION_PIN, INPUT_PULLUP);
    attachPCINT(digitalPinToPCINT(RESET_DETECTION_PIN), checkFactoryReset, FALLING);
  }

  void loop() {
    task.loop();
    digitalWrite(LED_BUILTIN, digitalRead(RESET_DETECTION_PIN));
  }
}
