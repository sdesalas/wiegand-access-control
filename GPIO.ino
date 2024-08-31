
#define GPIO_FACTORY_RESET 0 // "FLASH" pin
#define GPIO_RESET_DELAY (5*SECOND) // Press for 5 secs before flashing
#define GPIO_STATUS_LED 12 // (GPIO12 = D6)
#define GPIO_READER_D0 5 // (GPIO5 = D1)
#define GPIO_READER_D1 4 // (GPIO4 = D2)
#define GPIO_DOOR_RELAY 14 // (GPIO14 = D5)

void ICACHE_RAM_ATTR GPIO_factoryReset() {
  Serial.println("FLASH pin pressed.");
  // Check if still pressed 5 seconds later
  resetCheck.once_ms_scheduled(GPIO_RESET_DELAY, []() {
    if (digitalRead(GPIO_FACTORY_RESET) == LOW) { // LOW = pressed (INPUT_PULLUP)
      GPIO_blink(GPIO_STATUS_LED, 10, 100);
      Serial.println("WARNING! Factory reset detected.");
      Serial.println("Resetting in 5 seconds..");
      GPIO_blink(GPIO_STATUS_LED, 5, SECOND);
      Settings_reset();
      Metrics_reset();
      ESP.restart();
    } else {
      Serial.println("FALSE ALARM. Skipping factory reset.");
    };
  });
}

void GPIO_init() {
  Serial.print("GPIO_init()");
  // Initialize Pins
  pinMode(GPIO_READER_D0, INPUT); 
  pinMode(GPIO_READER_D1, INPUT);
  pinMode(GPIO_STATUS_LED, OUTPUT);
  pinMode(GPIO_DOOR_RELAY, OUTPUT);
  pinMode(GPIO_FACTORY_RESET, INPUT_PULLUP); // DO NOT CHANGE, "FLASH" pin starts HIGH
  digitalWrite(GPIO_STATUS_LED, 0);
  digitalWrite(GPIO_DOOR_RELAY, 0);
  // Add factory reset interrupt
  attachInterrupt(digitalPinToInterrupt(GPIO_FACTORY_RESET), GPIO_factoryReset, FALLING);
  // Initialize sensors
  gpio_sensors.begin();
  // Locate the devices on the bus:
  Serial.println("Locating devices.");
  Serial.print("-> ");
  gpio_device_count = gpio_sensors.getDeviceCount();
  Serial.print(gpio_device_count);
  Serial.println(" devices");
  delay(1000);
}

void GPIO_blink(int times) {
  GPIO_blink(times, 500);
}

void GPIO_blink(int times, int pause) {
  GPIO_blink(GPIO_BLUE_LED, times, pause);
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
