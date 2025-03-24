#include "Constants.h"
#include "Door.h"
#include "lib/AsyncTask.h"

#define DOOR1_PIN_D0 7 // D7 GREEN "D0"
#define DOOR1_PIN_D1 8 // D8 WHITE "D1"
#define DOOR1_LED 3 // 
#define DOOR1_BUZ 4 // 
#define DOOR1_RELAY 5 // 

Door Door1 = NULL;
AsyncTask Door1_task;

// Door 1 interrupts
void Door1_ISR_D0() { Door1.reader.ReadD0(); }
void Door1_ISR_D1() { Door1.reader.ReadD1(); }

// Door 1 flashing (non-blocking)
byte Door1_flashingState = 0;
unsigned int Door1_flashingTaskId = 0;
void Door1_startFlashing() {
  Serial.println("Door1_startFlashing()");
  if (Door1_flashingTaskId > 0) return;
  Door1_flashingState = 0;
  Door1_flashingTaskId = Door1_task.repeat([]() {
    Door1_flashingState = Door1.isConnected ? (Door1_flashingState + 1) % 2 : 0;
    // Serial.print("Door1_flashingState=");
    // Serial.println(Door1_flashingState);
    digitalWrite(DOOR1_LED, Door1_flashingState);
  }, 500);
}

void Door1_stopFlashing() {
  Serial.println("Door1_stopFlashing()");
  Door1_task.remove(Door1_flashingTaskId);
  Door1_flashingTaskId = 0;
}

void Door1_init() {
  Serial.begin(9600);
  Serial.println();
  delay(500);
  
  Serial.println("Starting Door1..");
  Door1 = Door(1);
  Door1.inputs(DOOR1_PIN_D0, DOOR1_PIN_D1, Door1_ISR_D0, Door1_ISR_D1);
  Door1.outputs(DOOR1_LED, DOOR1_BUZ, DOOR1_RELAY);
  Door1.actions->startFlashing = Door1_startFlashing;
  Door1.actions->stopFlashing = Door1_stopFlashing;
  Door1.start();
  Serial.println("Ready for input..");
}

unsigned long Door1_loop() {
  Door1.check();
  // Task scheduler has lower priority
  // run *after* handling card checks.
  if (millis() % 3 == 0) {
    Door1_task.loop(); 
  }
  // if(Door1.reader.available())
  // {
  //   unsigned long code = Door1.reader.getCode();
  //   Serial.print("keypadReader WG HEX = ");
  //   Serial.print(code, HEX);
  //   Serial.print(", DECIMAL = ");
  //   Serial.print(code);
  //   Serial.print(", Type W");
  //   Serial.println(Door1.reader.getWiegandType());
  //   Serial.println(); 
  //   return code;
  // }
}
