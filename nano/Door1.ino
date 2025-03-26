#include "Constants.h"
#include "Door.h"

#define DOOR1_PIN_D0 7 // D7 GREEN "D0"
#define DOOR1_PIN_D1 8 // D8 WHITE "D1"
#define DOOR1_LED 3 // BLUE CABLE
#define DOOR1_BUZ 4 // YELLOW CABLE
#define DOOR1_RELAY 5 // DOOR SSR

Door Door1 = Door(1);

// Door 1 interrupts
void Door1_ISR_D0() { Door1.reader.ReadD0(); }
void Door1_ISR_D1() { Door1.reader.ReadD1(); }

void Door1_init() {
  Serial.println("Starting Door1..");
  Door1.inputs(DOOR1_PIN_D0, DOOR1_PIN_D1, Door1_ISR_D0, Door1_ISR_D1);
  Door1.outputs(DOOR1_LED, DOOR1_BUZ, DOOR1_RELAY);
  Door1.start();
  Serial.println("Ready for input..");
}

unsigned long Door1_loop() {
  Door1.loop();
}
