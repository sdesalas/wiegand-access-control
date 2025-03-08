#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <Ticker.h>
#include <ESP8266WiFi.h>
#include "lib/WiegandMultiReader.h"

#define SECOND 1000
#define MINUTE (60 * SECOND)
#define HOUR (60 * MINUTE)
#define DAY (24 * HOUR)
#define WEEK (7 * DAY)
#define YEAR (52 * WEEK)

void setup() {
  Board_init();
  GPIO_init();
  FS_init();
  delay(200);
  Settings_init();
  Wifi_init();
  WebServer_init();
  GPIO_blink(10, 100);
}

void loop() {
  // Do nothing here.
  // Its all handled with timers and interrupts.
}
