
#define WIFI_CONN_MAX_WAIT 12

IPAddress APIP(192, 168, 66, 1); // Admin IP
Ticker blinker;

void Wifi_init() {
  Serial.println("Initializing Wifi..");
  Wifi_createAP();
  if (Wifi_connect()) {
    Board_ntp();
    WiFi.disconnect(true);
  } else {
    Serial.println("Error! Cannot connect to fetch NTP time...");
    GPIO_blink(GPIO_STATUS_LED, 5, 500);
  }
}

void Wifi_blink() {
  GPIO_blink(WiFi.softAPgetStationNum() + 1, 100);
  digitalWrite(GPIO_STATUS_LED, WiFi.softAPgetStationNum() > 0 ? 1 : 0);
}

void Wifi_createAP() {
  const char* ssid = setting_wifi_ap_ssid.c_str();
  const char* password = setting_wifi_ap_password.c_str();
  Serial.printf("Creating AP '%s' with password '%s******'....\n", ssid, password[0] || '*');
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

boolean Wifi_connect() {
  const char* ssid = setting_wifi_ssid.c_str();
  const char* password = setting_wifi_password.c_str();
  Serial.printf("Connecting to Wifi on [%s] (using %s*****)", ssid, password[0] || '*');
  WiFi.begin(ssid, password);
  unsigned long start = millis(); 
  bool blink = 0;
  while (WiFi.status() != WL_CONNECTED) {
    blink = !blink;
    digitalWrite(GPIO_STATUS_LED, blink);
    delay(200); Serial.print(".");
    if (millis() - start > WIFI_CONN_MAX_WAIT) {
      Serial.printf("Unable to connect over %d seconds.", WIFI_CONN_MAX_WAIT);
      break;
    }
  }
  digitalWrite(GPIO_STATUS_LED, 0);
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected! IP address is: ");
    Serial.println(WiFi.localIP());
    return true;
  }
  return false;
}