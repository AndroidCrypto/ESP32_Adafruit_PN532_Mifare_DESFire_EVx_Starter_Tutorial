/*
  This is the accompanying sketch to two articles:
  How to work with a Mifare DESFire EVx NFC tag on an ESP32 with PN532 reader 
  (Starter tutorial part 1 and part 2) thaat explains how to work with a
  Mifare DESFire EVx NFC card.
  
  The communication with the card is done by an ESP32 connected to a
  PN532 NFC card reader that is driven by the Adafruit_PN532 library.

  Please note: the tutorials and this sketch are running all examples without
  any authentication (e.g. writing and reading files with 'free' access right).

  The included DESFire library is based on the phantastic work of Piotr Obst
  who created the library 'MFRC522_NTAG424DNA' (communication between a MFRC522
  NFC reader and the NTAG424DNA NFC card), so a lot of credits go to him.

  Created by AndroidCrpto (Michael Fehr 2025)
*/

// --------------------------------------------------------------
// Programm Information
const char *PROGRAM_VERSION = "ESP32 Adafruit PN532 DESFire Starter V02";

// --------------------------------------------------------------
// PN532 library

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>  // https://github.com/adafruit/Adafruit-PN532

// settings for ESP32 ST7789 1.9-inches TFT display
#define PN532_SCK (33)
#define PN532_MOSI (32)
#define PN532_SS (25)
#define PN532_MISO (34)
#define PN532_IRQ (-1)    // not connected
#define PN532_RESET (-1)  // not connected
// The VCC pin of the reader is connected to the 3.3V pin of the ESP32

Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

#include "ESP32_DESFire.h"  // this is the DESFire Starter library

ESP32_DESFire desfire(&nfc);

void printHex(byte *buffer, uint16_t bufferSize);

const char *DIVIDER = "-------------------------------------------------------------------------";

boolean success;
char scrBuf[60];                          // buffer for tft outputs
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
ESP32_DESFire::DF_StatusCode dfStatusCode;

byte *appData = new byte[128];  // used as input or output buffer
byte appLen = 128;
uint16_t appLenExt = 128;
byte appDataByte = (byte)0xFF;

#include "T01_Basic.h"  // Tutorial workflow

void nfcInitialization() {
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.print("Didn't find PN53x board, halting");
    while (1)
      ;  // halt
  }

  // Got ok data, print it out!
  Serial.print("Found chip PN5");
  Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. ");
  Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.');
  Serial.println((versiondata >> 8) & 0xFF, DEC);

  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
  nfc.setPassiveActivationRetries(0xFF);
}

void setup(void) {
  Serial.begin(115200);
  delay(500);
  Serial.println(PROGRAM_VERSION);

  nfcInitialization();

  Serial.printf("ESP32_DESFire library version: %d\n", desfire.DESFIRE_SIMPLE_LIBRARY_VERSION);

  Serial.println("Waiting for an ISO14443A card");
}

void loop(void) {

  // Wait for an ISO14443A type cards (Mifare, etc.).
  success = nfc.inListPassiveTarget();

  if (success) {
    Serial.println("Found a card!");

    run_T01_Basic_Handling();

    delay(2000);
  }
}

void printHex(byte *buffer, uint16_t bufferSize) {
  for (uint16_t i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void printHexShort(byte *buffer, uint16_t bufferSize) {
  for (uint16_t i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? "0" : "");
    Serial.print(buffer[i], HEX);
  }
}