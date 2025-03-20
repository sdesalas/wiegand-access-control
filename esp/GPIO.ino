#include "lib/WiegandMultiReader.h"

// Board = ESP8266 NodeMCUv3

#define GPIO_FACTORY_RESET 0 // "FLASH" pin
#define GPIO_RESET_DELAY (5*SECOND) // Press for 5 secs before flashing
#define GPIO_STATUS_LED 2// D4 (Boot: High. Fails if pulled low)
#define GPIO_STATUS_BUZZ 1// TX (Boot: High. Fails if pulled low)

#define GPIO_READER1_D0 5 // D1
#define GPIO_READER1_D1 4 // D2
#define GPIO_READER1_DOOR 13 // D7

#define GPIO_READER2_D0 14 // D5
#define GPIO_READER2_D1 12 // D6
#define GPIO_READER2_DOOR 15 // D8

Ticker resetCheck;
Ticker readerCheck;

WIEGAND cardReader1;
void ICACHE_RAM_ATTR cardReader1_D0() { cardReader1.ReadD0(); }
void ICACHE_RAM_ATTR cardReader1_D1() { cardReader1.ReadD1(); }

WIEGAND cardReader2;
void ICACHE_RAM_ATTR cardReader2_D0() { cardReader2.ReadD0(); }
void ICACHE_RAM_ATTR cardReader2_D1() { cardReader2.ReadD1(); }

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
      // Metrics_reset();
      ESP.restart();
    } else {
      Serial.println("FALSE ALARM. Skipping factory reset.");
    };
  });
}

void GPIO_init() {
  Serial.println("GPIO_init()");

  // Add status LED / BUZ
  pinMode(GPIO_STATUS_LED, OUTPUT);
  pinMode(GPIO_STATUS_BUZZ, OUTPUT);

  // Add reader 1
  pinMode(GPIO_READER1_D0, INPUT);
  pinMode(GPIO_READER1_D1, INPUT);
  pinMode(GPIO_READER1_DOOR, OUTPUT);
  digitalWrite(GPIO_READER1_DOOR, 0);
  attachInterrupt(digitalPinToInterrupt(GPIO_READER1_D0), cardReader1_D0, FALLING);
  attachInterrupt(digitalPinToInterrupt(GPIO_READER1_D1), cardReader1_D1, FALLING);
  
  // Add reader 2
  pinMode(GPIO_READER2_D0, INPUT);
  pinMode(GPIO_READER2_D1, INPUT);
  pinMode(GPIO_READER2_DOOR, OUTPUT);
  digitalWrite(GPIO_READER2_DOOR, 0);
  attachInterrupt(digitalPinToInterrupt(GPIO_READER2_D0), cardReader2_D0, FALLING);
  attachInterrupt(digitalPinToInterrupt(GPIO_READER2_D1), cardReader2_D1, FALLING);
  
  // Add factory reset
  pinMode(GPIO_FACTORY_RESET, INPUT_PULLUP); // DO NOT CHANGE, "FLASH" pin starts HIGH
  attachInterrupt(digitalPinToInterrupt(GPIO_FACTORY_RESET), GPIO_factoryReset, FALLING);

  // Start reading
  delay(1000);
  readerCheck.once_ms_scheduled(1000, GPIO_checkReaders);
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

void GPIO_checkReaders() {
  if(cardReader1.available())
  {
    Serial.print("cardReader1 WG HEX = ");
    Serial.print(cardReader1.getCode(),HEX);
    Serial.print(", DECIMAL = ");
    Serial.print(cardReader1.getCode());
    Serial.print(", Type W");
    Serial.println(cardReader1.getWiegandType());    
  }
  if(cardReader2.available())
  {
    Serial.print("cardReader2 WG HEX = ");
    Serial.print(cardReader2.getCode(),HEX);
    Serial.print(", DECIMAL = ");
    Serial.print(cardReader2.getCode());
    Serial.print(", Type W");
    Serial.println(cardReader2.getWiegandType());
  }
  // Status LEDs     (=> ON if device connected properly)
  // digitalWrite(GPIO_READER1_LED, digitalRead(GPIO_READER1_D0) & digitalRead(GPIO_READER1_D1));
  // digitalWrite(GPIO_READER2_LED, digitalRead(GPIO_READER2_D0) & digitalRead(GPIO_READER2_D1));
  // Schedule next run
  readerCheck.once_ms_scheduled(50, GPIO_checkReaders);
}