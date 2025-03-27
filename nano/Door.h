/**
 * Door access abstraction
 * 
 * Each `Door` has 1 x WIEGAND reader and 1x DOOR
 */

#include "Constants.h"
#include "lib/WiegandMultiReader.h"
#include "lib/AsyncTask.h"
#include "PinChangeInterrupt.h"
#include "Storage.h"
#include "Arduino.h"

#ifndef _DOOR_H_
#define _DOOR_H_

#define DOOR_MAX_COUNT 4
#define DOOR_ADMIN_TIMEOUT (20 * SECOND)
#define DOOR_MASTERCARD_ADD 0 // (position in storage)
#define DOOR_MASTERCARD_DELETE 1

class Door {
  public:
    Door(byte id);
    static void checkFlashing();
    static void checkExitAdmin();
    static Door *all[DOOR_MAX_COUNT];
    enum Mode { NORMAL, ADD, DELETE, RESET, INIT };
    void inputs(byte D0, byte D1, void(*ISR_D0)(void), void(*ISR_D1)(void));
    void outputs(byte LED, byte BUZ, byte RELAY);
    void start();
    void loop();
    void handle(unsigned long card);
    void setMode(Mode m);
    void flash();
    void flash(byte onOff);
    void open();
    void startFlashing(unsigned int interval = 500);
    void stopFlashing();
    void shortBeep(byte count = 1);
    void mediumBeep(byte count = 1);
    void longBeep(byte count = 1);
    void beep(byte count, unsigned int duration);
    WIEGAND reader;
    Mode mode;
    AsyncTask tasks;
    byte id;
    bool isConnected;
    bool isAdmin;
    bool isFlashing = false;
    unsigned long modeStart;
    unsigned int flashingTaskId;
    unsigned int checkAdminTaskId;
    byte PIN_D0;
    byte PIN_D1;
    byte PIN_LED;
    byte PIN_BUZ;
    byte PIN_RELAY;
};

WIEGAND reader;
Door *Door::all[DOOR_MAX_COUNT];
Door::Mode mode;
AsyncTask tasks;
byte ID;
bool isConnected = false;
bool isAdmin = false;
bool isFlashing = false;
byte flashBit = 0;
unsigned long modeStart;
unsigned int flashingTaskId;
unsigned int checkAdminTaskId;
byte PIN_D0;
byte PIN_D1;
byte PIN_LED;
byte PIN_BUZ;
byte PIN_RELAY;

void Door::checkFlashing() {
  for (byte i = 0; i < DOOR_MAX_COUNT; i++) {
    Door *door = Door::all[i];
    if (door && door->flashingTaskId > 0) {
      door->flash();
      return;
    }
  }
}

void Door::checkExitAdmin() {
  for (byte i = 0; i < DOOR_MAX_COUNT; i++) {
    Door *door = Door::all[i];
    if (door && door->isAdmin && door->checkAdminTaskId > 0) {
      Serial.println(door->checkAdminTaskId);
      door->checkAdminTaskId = 0;
      if (door->isFlashing) {
        door->stopFlashing();
        door->setMode(Mode::NORMAL);
      }
      return;
    }
  }
}

Door::Door(byte id) {
  ID = id;
  for (byte i = 0; i < DOOR_MAX_COUNT; i++) {
    if (Door::all[i] == NULL) {
      Door::all[i] = this;
      break;
    }
  }  
}

void Door::inputs(byte D0, byte D1, void(*ISR_D0)(void), void(*ISR_D1)(void)) {
  PIN_D0 = D0;
  PIN_D1 = D1;
  pinMode(D0, INPUT);
  pinMode(D1, INPUT);
  isConnected = digitalRead(PIN_D0) & digitalRead(PIN_D1);
  attachPCINT(digitalPinToPCINT(D0), ISR_D0, FALLING);
  attachPCINT(digitalPinToPCINT(D1), ISR_D1, FALLING);
}

void Door::outputs(byte LED, byte BUZ, byte RELAY) {
  PIN_LED = LED;
  PIN_BUZ = BUZ;
  PIN_RELAY = RELAY;
  pinMode(LED, OUTPUT);
  pinMode(BUZ, OUTPUT);
  pinMode(RELAY, OUTPUT);
}

void Door::start() {
  // Check if door can do card admin
  if (Storage::adminReader == 0) {
    // No admin door means factory reset
    setMode(Mode::INIT);
  } else {
    isAdmin = Storage::adminReader == ID;
    setMode(Mode::NORMAL);
  }
}

void Door::loop() {
  // Status LEDs (=> Reader light GREEN/ON if device connected properly)
  isConnected = digitalRead(PIN_D0) & digitalRead(PIN_D1);
  if (mode == Mode::NORMAL) digitalWrite(PIN_LED, isConnected);
  if (reader.available()) {
    handle(reader.getCode());
  }
  // Task scheduler has lower priority
  // run *after* handling card checks.
  if (millis() % 4 == 0) {
    tasks.loop();
  }
}

void Door::handle(unsigned long card) {
  Serial.print("Door::handle() -> ");
  Serial.println(card);
  if (!(card > 0)) return;

  // Prevent admin if door is not enabled
  int slot = Storage::find(card);
  Serial.print("slot is -> ");
  Serial.println(slot);
  if (!isAdmin && slot == DOOR_MASTERCARD_ADD) return;
  if (!isAdmin && slot == DOOR_MASTERCARD_DELETE) return;

  switch(mode) {

    // INIT MODE
    //
    // Ocurs after a factory reset.
    //
    // There are no stored master cards.
    // - The 1st card is the `Master ADD` card
    // - The 2nd card is the `Master DELETE` card
    //
    case Mode::INIT: {
      if (slot == -1) {
        Storage::save(card);
        if (Storage::cards == 1) {
          shortBeep();
        }
        if (Storage::cards >= 2) {
          Storage::setMasterReader(ID);
          setMode(Mode::NORMAL);
          shortBeep(2);
          stopFlashing();
          reboot();
        }
      } else {
        longBeep();
      }
      return;
    }

    // NORMAL MODE
    //
    // Normal operation. Checks known ID cards for access.
    //
    // - Card known:          Access Granted
    // - Card unknown:        Access Denied
    // - Master ADD card:     -> ADD MODE (admin doors)
    // - Master DELETE card:  -> DELETE MODE (admin doors)
    //
    case Mode::NORMAL: {
      if (slot < 0) {
        // Unknown card
        return;
      }
      else if (isAdmin && slot == DOOR_MASTERCARD_ADD) {
        setMode(Mode::ADD);
        shortBeep(2);
        return;
      }
      else if (isAdmin && slot == DOOR_MASTERCARD_DELETE) {
        setMode(Mode::DELETE);
        shortBeep(2);
        return;
      }
      else 
        // Note: Admin cards also open non-admin doors
        open();
      return;
    }

    // ADD MODE
    //
    // Adds tapped ID cards.
    //
    // - Card unknown:        Adds to storage + EXITs
    // - Card known:          -> Just EXITs
    // 
    case Mode::ADD: {
      if (slot < 0) {
        // Unknown card -> ADD + EXIT
        Storage::save(card);
        shortBeep(2);
        setMode(Mode::NORMAL);
        return;
      } else {
        // Otherwise -> JUST EXIT
        shortBeep(1);
        setMode(Mode::NORMAL);
        return;
      }
      return;
    }

    // DELETE MODE
    //
    // Deletes tapped ID cards.
    //
    // - Card known:          Removes from storage + EXITs
    // - Double-tap:          -> RESET MODE (Clears known ID cards, except for Master ADD/DELETE)
    // - Master ADD card:     -> Do nothing
    // - Master DELETE card:  -> Just EXITs
    // - Card unkown          -> Just EXITs
    // 
    case Mode::DELETE: {
      if (slot == DOOR_MASTERCARD_ADD) {
        // Do nothing
      } else if (slot == DOOR_MASTERCARD_DELETE && millis() - modeStart < 5 * SECOND) {
        // Double tap -> RESET
        shortBeep(5);
        Storage::reset();
        setMode(Mode::RESET);
        return;
      } else if (slot >= 0) {
        // Known card -> REMOVE + EXIT
        Storage::remove(card);
        shortBeep(2);
        setMode(Mode::NORMAL);
        return;
      } else {
        // Otherwise -> JUST EXIT
        shortBeep(2);
        setMode(Mode::NORMAL);
        return;
      }
      return;
    }

    // RESET MODE
    //
    // Clears all known ID cards (except Master ADD/DELETE)
    //
    // - Card unknown:        Adds to storage + EXITs
    // - Triple-tap:          -> FACTORY RESET (Clears all cards, including Master ADD/DELETE)
    // - Master DELETE card:  -> Just EXITs
    // 
    case Mode::RESET: {
      if (slot == DOOR_MASTERCARD_DELETE) {
        if (millis() - modeStart < 5 * SECOND) {
          // Triple tap -> FACTORY RESET
          Storage::factoryReset();
          shortBeep(10);
          reboot();
          return;
        } else {
          // Normal tap -> EXIT
          shortBeep(3);
          setMode(Mode::NORMAL);
          return;
        }
      } else if (slot < 0) {
        // Unknown card -> Adds + EXITs
        Storage::save(card);
        shortBeep();
        setMode(Mode::NORMAL);
        return;
      }
    }
    return;
  }
}

void Door::setMode(Mode m) {
  Serial.print("Door::setMode() -> ");
  switch(m) {
    case 0: Serial.println("NORMAL"); break;
    case 1: Serial.println("ADD"); break;
    case 2: Serial.println("DELETE"); break;
    case 3: Serial.println("RESET"); break;
    case 4: Serial.println("INIT"); break;
  }
  
  mode = m;
  modeStart = millis();

  if (checkAdminTaskId > 0) {
    checkAdminTaskId = 0;
    tasks.remove(checkAdminTaskId);
  }
  
  switch(mode) {
    case Mode::NORMAL:
      stopFlashing();
      return;
    case Mode::ADD:
    case Mode::DELETE:
    case Mode::RESET:
      checkAdminTaskId = tasks.once([]() {
        Door::checkExitAdmin();
      }, DOOR_ADMIN_TIMEOUT);
      startFlashing(400);
      return;
    case Mode::INIT:
      startFlashing(1000);
      return;
  }
}

void Door::flash() {
  flash(flashBit ? 0 : 1);
}

void Door::flash(byte onOff) {
  if (isConnected) {
    digitalWrite(PIN_LED, flashBit = onOff); 
  } else {
    digitalWrite(PIN_LED, LOW);
  }
}

void Door::open() {
  digitalWrite(PIN_RELAY, HIGH);
  delay(1*SECOND);
  digitalWrite(PIN_RELAY, LOW);
}

void Door::startFlashing(unsigned int interval = 500) {
  if (isFlashing) stopFlashing();
  flashingTaskId = tasks.repeat([]() {
    Door::checkFlashing();
  }, interval);
  isFlashing = true;
}

void Door::stopFlashing() {
  if (flashingTaskId > 0) {
    tasks.remove(flashingTaskId);
    flashingTaskId = 0;
    isFlashing = false;
    flash(false);
  }
}

void Door::shortBeep(byte count = 1) {
  beep(count, 50*MS);
}

void Door::mediumBeep(byte count = 1) {
  beep(count, 120*MS);
}

void Door::longBeep(byte count = 1) {
  beep(count, 1*SECOND);
}

void Door::beep(byte count, unsigned int duration) {
  while(count-- > 0) {
    digitalWrite(PIN_BUZ, HIGH);
    delay(duration);
    digitalWrite(PIN_BUZ, LOW);
    if (count > 0) delay(duration);
  }  
}

#endif
