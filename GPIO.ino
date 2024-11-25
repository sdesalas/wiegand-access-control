
#define GPIO_FACTORY_RESET 0 // "FLASH" pin
#define GPIO_RESET_DELAY (5*SECOND) // Press for 5 secs before flashing
#define GPIO_STATUS_LED 12 // (GPIO12 = D6)
#define GPIO_READER_D0 5 // (GPIO5 = D1)
#define GPIO_READER_D1 4 // (GPIO4 = D2)
#define GPIO_READER_RELAY 14 // (GPIO14 = D5)
#define GPIO_KEYPAD_D0 5 // (GPIO5 = D1)
#define GPIO_KEYPAD_D1 4 // (GPIO4 = D2)
#define GPIO_KEYPAD_RELAY 14 // (GPIO14 = D5)

// Interrupt signature depends on board type
#if defined(ESP8266)
  #define ISR ICACHE_RAM_ATTR
#elif defined(ESP32)
  #define ISR IRAM_ATTR
#else
  #define ISR
#endif

Ticker resetCheck;

WIEGAND keypadReader();
void ISR keypadReadD0() { keypadReader.ReadD0(); }
void ISR keypadReadD1() { keypadReader.ReadD1(); }
void ISR GPIO_factoryReset() {
  Serial.println("FLASH pin pressed.");
  // Check if still pressed 5 seconds later
  resetCheck.once_ms_scheduled(GPIO_RESET_DELAY, []() {
    if (digitalRead(GPIO_FACTORY_RESET) == LOW) { // LOW = pressed (INPUT_PULLUP)
      GPIO_blink(GPIO_STATUS_LED, 10, 100);
      Serial.println("WARNING! Factory reset detected.");
      Serial.println("Resetting in 5 seconds..");
      GPIO_blink(GPIO_STATUS_LED, 5, SECOND);
      Settings_reset();
      // Metrics_reset();
      ESP.restart();
    } else {
      Serial.println("FALSE ALARM. Skipping factory reset.");
    };
  });
}

void GPIO_init() {
  Serial.println("GPIO_init()");

  // Add door relay + status LED
  pinMode(GPIO_STATUS_LED, OUTPUT);
  pinMode(GPIO_DOOR_RELAY, OUTPUT);
  digitalWrite(GPIO_STATUS_LED, 0);
  digitalWrite(GPIO_DOOR_RELAY, 0);

  // Add wiegand keypad
  pinMode(GPIO_KEYPAD_D0, INPUT);
  pinMode(GPIO_KEYPAD_D1, INPUT);
  attachInterrupt(digitalPinToInterrupt(GPIO_KEYPAD_D0), keypadReadD0, FALLING);
  attachInterrupt(digitalPinToInterrupt(GPIO_KEYPAD_D1), keypadReadD1, FALLING);
  
  // Add factory reset
  pinMode(GPIO_FACTORY_RESET, INPUT_PULLUP); // DO NOT CHANGE, "FLASH" pin starts HIGH
  attachInterrupt(digitalPinToInterrupt(GPIO_FACTORY_RESET), GPIO_factoryReset, FALLING);

  delay(1000);
  Serial.println("Ready for input..");
}

void GPIO_blink(int times) {
  GPIO_blink(times, 500);
}

void GPIO_blink(int times, int pause) {
  GPIO_blink(GPIO_STATUS_LED, times, pause);
}

void GPIO_blink(int pin, int times, int pause) {
  for (int i = 0; i < times; i++)
  {
    digitalWrite(pin, 1);
    delay(pause/2);
    digitalWrite(pin, 0);
    delay(pause/2);
  }
}
