#include "lib/TinyScheduler.h"
#include "Door.h"

#define GPIO_reader1_PIN_D0 7 // D7 GREEN "D0"
#define GPIO_reader1_PIN_D1 8 // D8 WHITE "D1"
#define GPIO_reader1_LED 3 // 
#define GPIO_reader1_BUZ 4 // 
#define GPIO_reader1_RELAY 5 // 

Door GPIO_door1(1);
TinyScheduler scheduler = TinyScheduler::millis();

void GPIO_door1_ISR_D0() { GPIO_door1.reader.ReadD0(); }
void GPIO_door1_ISR_D1() { GPIO_door1.reader.ReadD1(); }

void GPIO_init() {
  Serial.begin(9600);
  Serial.println();
  delay(500);
  
  Serial.println("Starting GPIO..");
  GPIO_door1.inputs(GPIO_reader1_PIN_D0, GPIO_reader1_PIN_D1, GPIO_door1_ISR_D0, GPIO_door1_ISR_D1);
  GPIO_door1.outputs(GPIO_reader1_LED, GPIO_reader1_BUZ, GPIO_reader1_RELAY);
  Serial.println("Ready for input..");

  scheduler.every(1000, []() {
    Serial.println("Checking GPIO Door 1");
    GPIO_door1.check();
  });
}

unsigned long GPIO_checkReaders() {
  // if(GPIO_door1.reader.available())
  // {
  //   unsigned long code = GPIO_door1.reader.getCode();
  //   Serial.print("keypadReader WG HEX = ");
  //   Serial.print(code, HEX);
  //   Serial.print(", DECIMAL = ");
  //   Serial.print(code);
  //   Serial.print(", Type W");
  //   Serial.println(GPIO_door1.reader.getWiegandType());
  //   Serial.println(); \
  //   return code;
  // }
  scheduler.loop();
}
