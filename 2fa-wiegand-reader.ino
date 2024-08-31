#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <Ticker.h>

#define SECOND 1000
#define MINUTE (60 * SECOND)
#define HOUR (60 * MINUTE)

// "wifi" options
String setting_wifi_ssid = "";
String setting_wifi_password = "";
boolean setting_wifi_hidden = false;

void setup() {
  Board_init();
  GPIO_init();
  FS_init();
  delay(200);
  Settings_init();
}

void loop() {
  // Do nothing here.
  // Its all handled with timers and interrupts.
}
