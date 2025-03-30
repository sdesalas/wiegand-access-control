#include "Constants.h"
#include "Door.h"

#define DOOR2_PIN_D0 7 // GREEN "D0"
#define DOOR2_PIN_D1 8 // WHITE "D1"
#define DOOR2_LED 9 // BLUE CABLE
#define DOOR2_BUZ 10 // YELLOW CABLE
#define DOOR2_RELAY 11 // DOOR OPENING

Door door2 = Door(2);

// Door 1 interrupts
void Door2_ISR_D0() { door2.reader.ReadD0(); }
void Door2_ISR_D1() { door2.reader.ReadD1(); }

void Door2_init() {
  Serial.println("Starting Door2..");
  door2.inputs(DOOR2_PIN_D0, DOOR2_PIN_D1, Door2_ISR_D0, Door2_ISR_D1);
  door2.outputs(DOOR2_LED, DOOR2_BUZ, DOOR2_RELAY);
  door2.start();
  Serial.println("Ready for input..");
}

unsigned long Door2_loop() {
  door2.loop();
}
