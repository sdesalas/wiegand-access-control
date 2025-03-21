#include "PinChangeInterrupt.h"
#include "WiegandMultiReader.h"

#define GPIO_reader1_PIN_D0 7 // D7 GREEN "D0"
#define GPIO_reader1_PIN_D1 8 // D8 WHITE "D1"
#define GPIO_reader1_PIN_LED 3 // 
#define GPIO_reader1_PIN_BUZ 4 // 

WIEGAND GPIO_reader1;

void GPIO_reader1_readD0() {
  GPIO_reader1.ReadD0();
}
void GPIO_reader1_readD1() {
  GPIO_reader1.ReadD1();
}

void GPIO_init() {
  Serial.begin(9600);
  Serial.println();
  delay(500);
  
  Serial.println("Starting GPIO..");
  pinMode(GPIO_reader1_PIN_D0, INPUT);
  pinMode(GPIO_reader1_PIN_D1, INPUT);
  pinMode(GPIO_reader1_PIN_LED, OUTPUT);
  digitalWrite(GPIO_reader1_PIN_LED, LOW);
  
  // Note that if connected properly, the wiegand device will run both input
  // pins D0 and D1 as HIGH, so we want to capture falling voltages.
  attachPCINT(digitalPinToPCINT(GPIO_reader1_PIN_D0), GPIO_reader1_readD0, FALLING);
  attachPCINT(digitalPinToPCINT(GPIO_reader1_PIN_D1), GPIO_reader1_readD1, FALLING);
  Serial.println("Ready for input..");
}

void GPIO_checkReaders() {
  if(GPIO_reader1.available())
  {
    Serial.print("keypadReader WG HEX = ");
    Serial.print(GPIO_reader1.getCode(),HEX);
    Serial.print(", DECIMAL = ");
    Serial.print(GPIO_reader1.getCode());
    Serial.print(", Type W");
    Serial.println(GPIO_reader1.getWiegandType());    
  }
  // Status LEDs (=> ON if device connected properly)
  digitalWrite(GPIO_reader1_PIN_LED, digitalRead(GPIO_reader1_PIN_D0) & digitalRead(GPIO_reader1_PIN_D1));
}
