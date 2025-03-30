#include "Constants.h"
#include "Door.h"

#define DOOR1_PIN_D0 5 // GREEN "D0"
#define DOOR1_PIN_D1 6 // WHITE "D1"
#define DOOR1_LED 2 // BLUE CABLE
#define DOOR1_BUZ 3 // YELLOW CABLE
#define DOOR1_RELAY 4 // DOOR OPENING

Door door1 = Door(1);

// Door 1 interrupts
void Door1_ISR_D0() { door1.reader.ReadD0(); }
void Door1_ISR_D1() { door1.reader.ReadD1(); }

void Door1_init() {
  Serial.println("Starting Door1..");
  door1.inputs(DOOR1_PIN_D0, DOOR1_PIN_D1, Door1_ISR_D0, Door1_ISR_D1);
  door1.outputs(DOOR1_LED, DOOR1_BUZ, DOOR1_RELAY);
  door1.start();
  Serial.println("Ready for input..");
}

unsigned long Door1_loop() {
  door1.loop();
}
