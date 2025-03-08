
#define WIFI_CONN_MAX_WAIT 12*SECOND

IPAddress APIP(192, 168, 66, 1); // Admin IP
Ticker blinker;
// Ticker ntpFetcher;

void Wifi_init() {
  Serial.println("Initializing Wifi..");
  // Wifi_initNtp();
  Wifi_createAP();
}

void Wifi_blink() {
  byte clients = WiFi.softAPgetStationNum();
  // Serial.printf("%d AP clients\n", clients);
  GPIO_blink(clients + 1, 100);
  // digitalWrite(GPIO_STATUS_LED, uint8_t > 0 ? 1 : 0);
}

void Wifi_createAP() {
  const char* ssid = setting_wifi_ap_ssid.c_str();
  const char* password = setting_wifi_ap_password.c_str();
  Serial.printf("Creating AP '%s' with password '%s'....\n", ssid, password);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(APIP, APIP, IPAddress(255, 255, 255, 0));
  if (WiFi.softAP(ssid, password, 1, setting_wifi_ap_hidden)) {
    Serial.println("AP Created!");
    // Blink every 5 seconds
    blinker.attach_ms_scheduled(5005, Wifi_blink);
  } else {
    Serial.println("Error! Cannot create AP. Reboot in 5 seconds..");
    GPIO_blink(GPIO_STATUS_LED, 5, SECOND);
    ESP.restart();
  }
}

// long ntpRetryMs = 10*SECOND;
// void Wifi_initNtp() {
//     // The board itself does not have a CMOS clock.
//     // Getting accurate reading from NTP Server is important because
//     // access logs need a timestamp in order to be meaningful.
//     // Keep trying for 30 minutes (exponential backoff).
//   if (Wifi_connect() && Board_ntp()) {
//     WiFi.disconnect(true);
//   } else {
//     Serial.println("Error! Cannot connect for NTP time.");
//     GPIO_blink(GPIO_STATUS_LED, 5, 250);
//     if (ntpRetryMs < 15*MINUTE) {
//       Serial.printf("Retry in %d seconds...\n", ntpRetryMs/SECOND);
//       ntpFetcher.once_ms_scheduled(ntpRetryMs, Wifi_initNtp);
//       ntpRetryMs *= 2; // --> Exponential backoff
//     }
//   }
// }

// boolean Wifi_connect() {
//   const char* ssid = setting_wifi_ssid.c_str();
//   const char* password = setting_wifi_password.c_str();
//   Serial.printf("Connecting to Wifi [%s] (using '%s')...\n", ssid, password);
//   WiFi.begin(ssid, password);
//   unsigned long start = millis(); 
//   bool blink = 0;
//   while (WiFi.status() != WL_CONNECTED) {
//     blink = !blink;
//     digitalWrite(GPIO_STATUS_LED, blink);
//     delay(200); Serial.print(".");
//     if (millis() - start > WIFI_CONN_MAX_WAIT) {
//       Serial.printf("\nUnable to connect over %d seconds.", WIFI_CONN_MAX_WAIT/SECOND);
//       break;
//     }
//   }
//   digitalWrite(GPIO_STATUS_LED, 0);
//   Serial.println();
//   if (WiFi.status() == WL_CONNECTED) {
//     Serial.print("Connected! IP address is: ");
//     Serial.println(WiFi.localIP());
//     return true;
//   }
//   return false;
// }