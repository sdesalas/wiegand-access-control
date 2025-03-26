/**
 * Door access abstraction
 * 
 * Each `Door` has 1 x WIEGAND reader and 1x DOOR
 */

#include "Constants.h"
#include "lib/WiegandMultiReader.h"
#include "PinChangeInterrupt.h"
#include "Storage.h"
#include "Arduino.h"

#ifndef _DOOR_H_
#define _DOOR_H_

#define DOOR_MASTERCARD_ADD 0 // (position in storage)
#define DOOR_MASTERCARD_DELETE 1

struct DoorEvents {
  void(*startFlashing)(void);
  void(*stopFlashing)(void);
};

class Door {
  public:
    Door(byte id);
    enum Mode { NORMAL, ADD, DELETE, RESET, INIT };
    void inputs(byte D0, byte D1, void(*ISR_D0)(void), void(*ISR_D1)(void));
    void outputs(byte LED, byte BUZ, byte RELAY);
    void start();
    void check();
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
    byte id;
    bool isConnected;
    bool isAdmin;
    byte flashOnOff;
    Mode mode;
    unsigned long modeStart;
    unsigned int flashingInterval;
    byte PIN_D0;
    byte PIN_D1;
    byte PIN_LED;
    byte PIN_BUZ;
    byte PIN_RELAY;
    DoorEvents *on;
};

WIEGAND reader;
byte ID;
bool isConnected = false;
bool isAdmin = false;
byte flashOnOff = 0;
Door::Mode mode;
unsigned long modeStart;
unsigned int flashingInterval;
byte PIN_D0;
byte PIN_D1;
byte PIN_LED;
byte PIN_BUZ;
byte PIN_RELAY;
DoorEvents *on;

Door::Door(byte id) {
  if (id == 0) return; // ERR
  ID = id;
  on->startFlashing = []() {};
  on->stopFlashing = []() {};
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

void Door::check() {
  // Status LEDs (=> Reader light GREEN/ON if device connected properly)
  isConnected = digitalRead(PIN_D0) & digitalRead(PIN_D1);
  if (mode == Mode::NORMAL) digitalWrite(PIN_LED, isConnected);
  // Serial.println("Checking GPIO Door 1");
  if (reader.available()) {
    handle(reader.getCode());
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
          Serial.println("Master ADD Saved!");
          shortBeep();
        }
        if (Storage::cards >= 2) {
          Serial.println("Master DELETE Saved!");
          Storage::setMasterReader(ID);
          setMode(Mode::NORMAL);
          shortBeep(2);
          stopFlashing();
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
    // - Card known:          Nothing
    // - Card unknown:        Adds to storage + EXITs
    // - Master ADD card:     -> Just EXITs
    // 
    case Mode::ADD: {
      if (slot < 0) {
        // Unknown card -> ADD + EXIT
        Storage::save(card);
        shortBeep(2);
        setMode(Mode::NORMAL);
        return;
      } else if (slot == DOOR_MASTERCARD_ADD) {
        // Master card -> JUST EXIT
        shortBeep(1);
        setMode(Mode::NORMAL);
        return;
      }
    }

    // DELETE MODE
    //
    // Deletes tapped ID cards.
    //
    // - Card known:          Removes from storage + EXITs
    // - Card unknown:        Nothing
    // - Double-tap:          -> RESET MODE (Clears known ID cards, except for Master ADD/DELETE)
    // - Master DELETE card:  -> Just EXITs
    // 
    case Mode::DELETE: {
      if (slot == DOOR_MASTERCARD_ADD) {
        // Do nothing
      } else if (slot == DOOR_MASTERCARD_DELETE) {
        if (millis() - modeStart < 5 * SECOND) {
          // Double tap -> RESET
          shortBeep(5);
          Storage::reset();
          setMode(Mode::RESET);
          return;
        } else {
          // Normal tap -> JUST EXIT
          shortBeep(2);
          setMode(Mode::NORMAL);
          return;
        }
      } else if (slot >= 0) {
        // Known card -> REMOVE + EXIT
        Storage::remove(card);
        shortBeep(2);
        setMode(Mode::NORMAL);
        return;
      }
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
          setMode(Mode::INIT);
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
  switch(mode) {
    case Mode::NORMAL:
      stopFlashing();
      return;
    case Mode::DELETE:
      startFlashing(250);
      return;
    case Mode::RESET:
      startFlashing(50);
      return;
    default:
      startFlashing(500);
      return;
  }
}

void Door::flash() {
  flash(flashOnOff ? 0 : 1);
}

void Door::flash(byte onOff) {
  if (isConnected) {
    digitalWrite(PIN_LED, flashOnOff = onOff); 
  } else {
    digitalWrite(PIN_LED, LOW);
  }
}

void Door::open() {
  digitalWrite(PIN_RELAY, HIGH);
  delay(2*SECOND);
  digitalWrite(PIN_RELAY, LOW);
}

void Door::startFlashing(unsigned int interval = 500) {
  flashingInterval = interval;
  on->startFlashing();  
}

void Door::stopFlashing() {
  on->stopFlashing();
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
