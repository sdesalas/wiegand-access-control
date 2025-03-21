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

#define START_ADDRESS 300 // This number < 800 should be rotated for reduced EEPROM wear.
#define MAX_CARDS 32

byte Storage_cards = 0;
byte Storage_primaryReader = 0;

void Storage_init() {
  Storage_cards = EEPROM.read(START_ADDRESS);
  Storage_primaryReader = EEPROM.read(START_ADDRESS + 1);
}

void Storage_reset() {
  Storage_cards = 0;
  Storage_primaryReader = 0;
  EEPROM.write(START_ADDRESS, Storage_cards);
  EEPROM.write(START_ADDRESS + 1, Storage_primaryReader);
}

byte Storage_findCard(long id) {
  for(byte i = 0; i < MAX_CARDS; i++) {
    if (i >= Storage_cards) break; // no more cards
    if (Storage_readCard(i) == id) return i;
  }
  return -1;
}

long Storage_readCard(byte index) {
  if (index > Storage_cards) return 0;

  // read 4 bytes
  int address = START_ADDRESS + 2 + (index * 4);
  long b1 = EEPROM.read(index);
  long b2 = EEPROM.read(index + 1);
  long b3 = EEPROM.read(index + 2);
  long b4 = EEPROM.read(index + 3);
  
  // 4 bytes -> long
  return ((b1 << 0) & 0xFF) + ((b2 << 8) & 0xFFFF) + ((b3 << 16) & 0xFFFFFF) + ((b4 << 24) & 0xFFFFFFFF);
}

bool Storage_saveCard(long id) {
  // pick next avavailable slot
  return Storage_writeCard(Storage_cards, id);
}

bool Storage_writeCard(byte index, long id) {
  // check the limit
  if (index > MAX_CARDS) return false;

  // long -> 4 bytes
  byte b1 = (id & 0xFF);
  byte b2 = ((id >> 8) & 0xFF);
  byte b3 = ((id >> 16) & 0xFF);
  byte b4 = ((id >> 24) & 0xFF);

  // Write to eeprom
  int address = START_ADDRESS + 2 + (index * 4);
  EEPROM.write(address, b1);
  EEPROM.write(address + 1, b2);
  EEPROM.write(address + 2, b3);
  EEPROM.write(address + 3, b4);

  // We have one more card in memory
  Storage_cards++;
  EEPROM.write(START_ADDRESS, Storage_cards);
  return true;
}
