/**
 * EEPROM CARD storage
 * 
 * Store up to 32 Wiegand 26 cards (32 bits / 4 bytes)
 * 
 * Note that Arduino Nano/Uno have 1024 EEPROM bytes.
 * Around 100,000 write cycles so we should be careful wear rotation.
 * 
 * First byte is number of cards in storage.
 * Each 4 bytes after contains is a 10-digit card ID.
 * 
 * Each card is 12ms read so max time to find a card is ~1200ms.
 */
#include "EEPROM.h"

#ifndef _STORAGE_H_
#define _STORAGE_H_

// Storage location in EEPROM
// This number < 800 should be rotated for reduced wear.
#define STORAGE_META_ADDRESS 300 
#define STORAGE_START_ADDRESS (STORAGE_META_ADDRESS + 4)
#define STORAGE_MAX_CARDS 32

namespace Storage {

  // metadata
  byte cards = 0;
  byte adminReader = 0;
  
  void init() {
    cards = EEPROM.read(STORAGE_META_ADDRESS);
    adminReader = EEPROM.read(STORAGE_META_ADDRESS + 1);
  }
  
  void reset() {
    cards = 2;
    EEPROM.write(STORAGE_META_ADDRESS, cards);
  }
  
  void factoryReset() {
    cards = 0;
    adminReader = 0;
    EEPROM.write(STORAGE_META_ADDRESS, cards);
    EEPROM.write(STORAGE_META_ADDRESS + 1, adminReader);
  }
  
  void setMasterReader(byte door) {
    adminReader = door;
    EEPROM.write(STORAGE_META_ADDRESS + 1, adminReader);
  }

  /** Returns a 4-byte card number at slot */
  unsigned long read(byte slot) {
    if (slot > cards) return 0;
  
    // read 4 bytes
    int address = STORAGE_START_ADDRESS + (slot * 4);
    long b1 = EEPROM.read(slot);
    long b2 = EEPROM.read(slot + 1);
    long b3 = EEPROM.read(slot + 2);
    long b4 = EEPROM.read(slot + 3);
    
    // 4 bytes -> unsigned long
    return ((b1 << 0) & 0xFF) + ((b2 << 8) & 0xFFFF) + ((b3 << 16) & 0xFFFFFF) + ((b4 << 24) & 0xFFFFFFFF);
  }
  
  /** Returns position of card in storage */
  int find(unsigned long card) {
    for(byte i = 0; i < STORAGE_MAX_CARDS; i++) {
      if (i >= cards) break; // no more cards
      if (read(i) == card) return i;
    }
    return -1;
  }

  /** Writes a card number at a specified slot */
  bool write(byte slot, unsigned long card) {
    // check the limit
    if (slot > STORAGE_MAX_CARDS) return false;
  
    // long -> 4 bytes
    byte b1 = (card & 0xFF);
    byte b2 = ((card >> 8) & 0xFF);
    byte b3 = ((card >> 16) & 0xFF);
    byte b4 = ((card >> 24) & 0xFF);
  
    // Write to eeprom
    int address = STORAGE_START_ADDRESS + (slot * 4);
    EEPROM.write(address, b1);
    EEPROM.write(address + 1, b2);
    EEPROM.write(address + 2, b3);
    EEPROM.write(address + 3, b4);
  
    // We have one more card in storage
    EEPROM.write(STORAGE_META_ADDRESS, ++cards);
    return true;
  }  
  
  /** Saves a card in next available slot, returns the slot */
  int save(unsigned long card) {
    if (write(cards, card)) {
      return cards;
    }
    return -1;
  }

  /** Removes a card, returns true if successful */
  bool remove(unsigned long card) {
    int slot = find(card);
    if (slot < 0) return false;

    // Find the location of the card then
    // shift all bytes 4 positions to the left
    // until we reach current number of cards.
    int from = STORAGE_START_ADDRESS + (slot * 4);
    int to = STORAGE_START_ADDRESS + (cards * 4);
    for (int address = from; address < to; address++) {
      EEPROM.write(address, EEPROM.read(address+4));
    }

    // We have one less card in storage
    EEPROM.write(STORAGE_META_ADDRESS, --cards);
    return true;
  }

}

#endif
