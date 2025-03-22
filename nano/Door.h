/**
 * Door access abstraction
 * 
 * Each `Door` has 1 x WIEGAND reader and 1x DOOR
 */

#include "Constants.h"
#include "WiegandMultiReader.h"
#include "Storage.h"
#include "Arduino.h"

#ifndef _DOOR_H_
#define _DOOR_H_

#define DOOR_MASTERCARD_ADD 0 // (position in storage)
#define DOOR_MASTERCARD_DELETE 1

class Door {
  public:
    Door(byte id);
    enum Mode { NORMAL, ADD, DELETE, RESET, INIT };
    void initInput(byte D0, byte D1, void(*ISR_D0)(void), void(*ISR_D1)(void));
    void initOutput(byte LED, byte BUZ, byte RELAY);
    void check();
    void handle(unsigned long card);
    void setMode(Mode m);
    void open();
    void startFlashing();
    void stopFlashing();
    void shortBeep(byte count);
    void longBeep(byte count);
    WIEGAND reader;
    bool isAdmin;
    Mode mode;
    unsigned long modeStart;
    byte PIN_D0;
    byte PIN_D1;
    byte PIN_LED;
    byte PIN_BUZ;
    byte PIN_RELAY;
};

WIEGAND reader;
bool isAdmin = false;
Door::Mode mode;
unsigned long modeStart;
byte PIN_D0;
byte PIN_D1;
byte PIN_LED;
byte PIN_BUZ;
byte PIN_RELAY;

Door::Door(byte id) {
  if (id == 0) return; // ERR
  // Check if door can do card admin
  if (Storage::adminReader == 0) {
    // No admin door means factory reset
    setMode(Mode::INIT);
  } else {
    isAdmin = Storage::adminReader == id;
  }
}

void Door::initInput(byte D0, byte D1, void(*ISR_D0)(void), void(*ISR_D1)(void)) {
  PIN_D0 = D0;
  PIN_D1 = D1;
  pinMode(D0, INPUT);
  pinMode(D1, INPUT);
  attachPCINT(digitalPinToPCINT(D0), ISR_D0, FALLING);
  attachPCINT(digitalPinToPCINT(D1), ISR_D1, FALLING);
}

void Door::initOutput(byte LED, byte BUZ, byte RELAY) {
  PIN_LED = LED;
  PIN_BUZ = BUZ;
  PIN_RELAY = RELAY;
  pinMode(LED, OUTPUT);
  pinMode(BUZ, OUTPUT);
  pinMode(RELAY, OUTPUT);
}

void Door::check() {
  // Status LEDs (=> Reader light GREEN/ON if device connected properly)
  digitalWrite(PIN_LED, digitalRead(PIN_D0) & digitalRead(PIN_D1));
  if (reader.available()) {
    unsigned long card = reader.getCode();
    handle(card);
  }
}

void Door::handle(unsigned long card) {
  if (!(card > 0)) return;

  // Prevent admin if door is not enabled
  byte slot = Storage::find(card);
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
      Storage::save(card);
      if (Storage::cards >= 2) {
        setMode(Mode::NORMAL);
        stopFlashing();
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
        longBeep(1);
        return;
      }
      else if (isAdmin && slot == DOOR_MASTERCARD_ADD) {
        setMode(Mode::ADD);
        shortBeep(1);
        return;
      }
      else if (isAdmin && slot == DOOR_MASTERCARD_DELETE) {
        setMode(Mode::DELETE);
        shortBeep(1);
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
    // - Card unknown:        Adds to storage
    // - Master ADD card:     -> Exits
    // 
    case Mode::ADD: {
      if (slot < 0) {
        // Unknown card
        Storage::save(card);
        shortBeep(1);
        return;
      } else if (slot == DOOR_MASTERCARD_ADD) {
        setMode(Mode::NORMAL);
        shortBeep(3);
        return;
      }
    }

    // DELETE MODE
    //
    // Deletes tapped ID cards.
    //
    // - Card known:          Removes to storage
    // - Card unknown:        Nothing
    // - Double-tap:          -> RESET MODE (Clears known ID cards, except for Master ADD/DELETE)
    // - Master DELETE card:  -> Exits
    // 
    case Mode::DELETE: {
      if (slot < 0) {
        // Unknown card
        Storage::remove(card);
        shortBeep(1);
        return;
      } else if (slot == DOOR_MASTERCARD_DELETE) {
        if (millis() - modeStart < 5 * SECOND) {
          // Double tap -> RESET
          setMode(Mode::RESET);
          Storage::reset();
          shortBeep(5);
          return;
        } else {
          // Normal tap -> EXIT
          setMode(Mode::NORMAL);
          shortBeep(3);
          return;
        }
      }
    }

    // RESET MODE
    //
    // Clears all known ID cards (except Master ADD/DELETE)
    //
    // - Card known:          Nothing
    // - Card unknown:        Adds to storage
    // - Triple-tap:          -> FACTORY RESET (Clears all cards, including Master ADD/DELETE)
    // - Master DELETE card:  -> Exits
    // 
    case Mode::RESET: {
      if (slot < 0) {
        // Unknown card
        Storage::save(card);
        shortBeep(1);
        return;
      } else if (slot == DOOR_MASTERCARD_DELETE) {
        if (millis() - modeStart < 5 * SECOND) {
          // Triple tap -> FACTORY RESET
          Storage::reset();
          Storage::factoryReset();
          shortBeep(10);
          setMode(Mode::NORMAL);
          return;
        } else {
          // Normal tap -> EXIT
          shortBeep(3);
          setMode(Mode::NORMAL);
          return;
        }
      }
    }
  }
}

void Door::setMode(Mode m) {
  mode = m;
  modeStart = millis();
  switch(mode) {
    case Mode::NORMAL:
      stopFlashing();
      return;
    default:
      startFlashing();
      return;
  }
}

void Door::open() {
  // Opens the door
}

void Door::startFlashing() {}

void Door::stopFlashing() {}

void Door::shortBeep(byte count) {}

void Door::longBeep(byte count) {}

#endif
