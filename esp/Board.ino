void Board_init() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("=================================");
  Serial.print("ESP.getChipId(): ");
  Serial.println(ESP.getChipId());
  Serial.print("ESP.getCoreVersion(): ");
  Serial.println(ESP.getCoreVersion());
  Serial.print("ESP.getSdkVersion(): ");
  Serial.println(ESP.getSdkVersion());
  Serial.print("ESP.getCpuFreqMHz(): ");
  Serial.println(ESP.getCpuFreqMHz());
  Serial.print("ESP.getSketchSize(): ");
  Serial.println(ESP.getSketchSize());
  Serial.print("ESP.getSketchMD5(): ");
  Serial.println(ESP.getSketchMD5().substring(0,6));
  Serial.print("ESP.getFreeSketchSpace(): ");
  Serial.println(ESP.getFreeSketchSpace());
  Serial.print("ESP.getFreeHeap(): ");
  Serial.println(ESP.getFreeHeap());
  Serial.print("ESP.getHeapFragmentation(): ");
  Serial.println(ESP.getHeapFragmentation());
  Serial.print("ESP.getFlashChipSize(): ");
  Serial.println(ESP.getFlashChipSize());
  Serial.print("ESP.getFlashChipSpeed(): ");
  Serial.print(ESP.getFlashChipSpeed() / 1000 / 1000);
  Serial.println("Mhz");
  Serial.println("=================================");
}

#define MINUTE_IN_SECS 60
#define HOUR_IN_SECS (60 * MINUTE_IN_SECS)
#define DAY_IN_SECS (24 * HOUR_IN_SECS)

String Board_getTime() {
  unsigned long seconds = Board_seconds();
  unsigned long d = (seconds / DAY_IN_SECS);
  unsigned long h = (seconds % DAY_IN_SECS) / HOUR_IN_SECS;
  String hh = h < 10 ? "0" + String(h) : String(h);
  unsigned long m = (seconds % HOUR_IN_SECS) / MINUTE_IN_SECS;
  String mm = m < 10 ? "0" + String(m) : String(m);
  unsigned long s = (seconds % MINUTE_IN_SECS);
  String ss = s < 10 ? "0" + String(s) : String (s);
  return  String(d) + "T" + hh + ":" + mm + ":" + ss;
}

// Seconds since the board was powered (up to 136 years).
unsigned int Board_overflows = 0;
unsigned long Board_lastMillis = 0;
unsigned long Board_seconds() {
  unsigned long ms = millis();
  if (ms < Board_lastMillis) {
    // NOTE: the `millis()` function result overflows and 
    // wrap-arounds back to 0 after 2^32 milliseconds (49.7 days)
    Board_overflows++;
  }
  Board_lastMillis = ms;
  unsigned long offset = Board_overflows * ((2 << 32) / 1000);
  return offset + (ms / 1000);
}