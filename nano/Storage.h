/**
 * EEPROM CARD storage
 * 
 * Store up to 32 Wiegand 26 cards (32 bits / 4 bytes)
 * 
 * Note that Arduino Nano/Uno have 1024 EEPROM bytes.
 * Around 100,000 write cycles so we should be careful wear rotation.
 * 
 * Storage is in 4-byte slots. First 4 bytes contain metadata.
 * Each 4 bytes after contains is a 10-digit card ID.
 * 
 * Each card is 12ms read (4 bytes / 3s each) so max time to find a card is ~0.4s.
 */
#include "EEPROM.h"

#ifndef _STORAGE_H_
#define _STORAGE_H_

#define DEBUG 0

// Storage location in EEPROM
// This number < 800 should be rotated for reduced wear.
#define STORAGE_META_ADDRESS 300 
#define STORAGE_START_ADDRESS (STORAGE_META_ADDRESS + 4)
#define STORAGE_MAX_CARDS 32

namespace Storage {

  // number of cards currently stored
  byte cards = 0;
  // which of the doors can adminster cards
  byte adminReader = 0;
  
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
    long b1 = EEPROM.read(address);
    long b2 = EEPROM.read(address + 1);
    long b3 = EEPROM.read(address + 2);
    long b4 = EEPROM.read(address + 3);
    
    // 4 bytes -> unsigned long
    return ((b1 << 0) & 0xFF) + ((b2 << 8) & 0xFFFF) + ((b3 << 16) & 0xFFFFFF) + ((b4 << 24) & 0xFFFFFFFF);
  }

  void init() {
    cards = EEPROM.read(STORAGE_META_ADDRESS);
    adminReader = EEPROM.read(STORAGE_META_ADDRESS + 1);

    #if DEBUG
    Serial.print("Storage::cards "); Serial.println(cards);
    Serial.print("Storage::adminReader "); Serial.println(adminReader);
    for (byte i = 0; i < cards; i++) {
      Serial.print("Card@"); Serial.print(i); Serial.print(":"); Serial.println(read(i));
    }
    #endif
  }
  
  /** Returns position of card in storage */
  int find(unsigned long card) {
    #if DEBUG
    Serial.print("Storage::find() -> ");
    Serial.println(card);
    #endif

    for(byte i = 0; i < STORAGE_MAX_CARDS; i++) {
      if (i >= cards) break; // no more cards
      unsigned long slot = read(i);

      #if DEBUG
      Serial.print(i);
      Serial.print(" -> ");
      Serial.println(slot);
      #endif

      if (slot == card) return i;
    }
    return -1;
  }

  /** Writes a card number at a specified slot */
  bool write(byte slot, unsigned long card) {
    #if DEBUG
    Serial.print("Storage::write() ");
    Serial.print(slot);
    Serial.print(" => ");
    Serial.println(card);
    #endif

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

    #if DEBUG
    Serial.print(address); Serial.print(" => "); Serial.println(b1);
    Serial.print(address + 1); Serial.print(" => "); Serial.println(b2);
    Serial.print(address + 2); Serial.print(" => "); Serial.println(b3);
    Serial.print(address + 3); Serial.print(" => "); Serial.println(b4);
    #endif
  
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
    #if DEBUG
    Serial.print("Storage::remove() ");
    Serial.println(card);
    #endif

    int slot = find(card);
    if (slot < 0) return false;

    // Find the location of the card then
    // shift all bytes 4 positions to the left
    // until we reach current number of cards.
    // Worst case, this could take a few seconds.
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
