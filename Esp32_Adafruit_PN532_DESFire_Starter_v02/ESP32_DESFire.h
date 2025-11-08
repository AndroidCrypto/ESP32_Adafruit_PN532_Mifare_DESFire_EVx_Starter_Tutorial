/**
 * A library for ESP32 and MFRC522 or PN532 NFC Reader modules.
 * There are just a minimum of commands implemented that are
 * good for a simple 'read and write' tutorial.
 * For authentication the modern 'AuthenticateEV2First' method is used.
 *
 * The library is largely based on the outstanding work of Piotr Obst in developing 
 * the MFRC522_NTAG424DNA library, so all credit goes to him.
 * https://github.com/Obsttube/MFRC522_NTAG424DNA. License: MIT. 
 *
 * The communication is based on the Adafruit_PN532 library:
 * https://github.com/adafruit/Adafruit-PN532 version 1.3.4
 * The library needs to get modified in one file: 'Adafruit_PN532.cpp'
 * In line 78 change one parameter
 * old: #define PN532_PACKBUFFSIZ 64 ///< Packet buffer size in bytes
 * new: #define PN532_PACKBUFFSIZ 255 ///< Packet buffer size in bytes
 *
 * Author: Michael Fehr (AndroidCrypto)
*/

/*
 * Known restrictions with this implementation
 * - all read and write data file operations are limited to 256 bytes, as the parameter is just a byte
 * - Don't use FULL/encrypted record files with record sizes > 32 bytes, as the reading requires a decryption
 *   that seem to write into not allocated memory areas. This can be a reason for crashes, so stay on 32 bytes please.
Change in Adafruit_PN532.cpp
#define PN532_PACKBUFFSIZ 64                ///< Packet buffer size in bytes
to #define PN532_PACKBUFFSIZ 253            ///< Packet buffer size in bytes
byte pn532_packetbuffer[PN532_PACKBUFFSIZ]; ///< Packet buffer used in various
                                            ///< transactions
*/

/**
 * Version Management 
 * V02 08.11.2025 Impl. StatusCode Debug Printer, Impl. Read and Write Data simplified
 * V01 07.11.2025 Initial programming, based on full library V50
*/

/*
Wiring of the RED PN532 NFC module to an ESP32 T-Display using SPI interface
Pins seen from chip side of the module from LEFT to RIGHT
PN532 - ESP32
SCK   - 15
MISO  - 12
MOSI  - 13 
SS/CS - 17
VCC   - 3.3V 
GND   - GND

all PINs are on the right side of the dev board, seen from display side and USB bottom

#define PN532_SCK (15)
#define PN532_MOSI (13)
#define PN532_SS (17)  // CS
#define PN532_MISO (12)
*/

#ifndef ESP32_DESFire_h
#define ESP32_DESFire_h

#include "Arduino.h"
#include "Adafruit_PN532.h"

class ESP32_DESFire {

public:

  /////////////////////////////////////////////////////////////////////////////////////
  // Contructors
  /////////////////////////////////////////////////////////////////////////////////////

  ESP32_DESFire(Adafruit_PN532* nfc);

  const uint8_t DESFIRE_SIMPLE_LIBRARY_VERSION = 02;
  bool COMM_DEBUG_PRINT = true;  // if true the send and received data is printed

// Note: just a minimal of commands are implemented
// Mifare DESFireCommands
#define DESFIRE_CREATE_APPLICATION (0xCA)
#define DESFIRE_SELECT_APPLICATION (0x5A)
#define DESFIRE_GET_VERSION (0x60)
#define DESFIRE_GET_FILE_SETTINGS (0xF5)
#define DESFIRE_GET_FREE_MEMORY (0x6E)
#define DESFIRE_GET_MORE_DATA (0xAF)
#define DESFIRE_CREATE_STANDARD_DATA_FILE (0xCD)
#define DESFIRE_READ_DATA_FILE (0xBD)
#define DESFIRE_WRITE_DATA_FILE (0x8D)
#define DESFIRE_SV2_OK (0x00)

  enum DF_StatusCode : byte {
    DF_STATUS_OK = 0,              // Success (and 0x9000, 0x9100 - OPERATION_OK / Successful operaton)
    DF_STATUS_ERROR = 1,           // Error in communication
    DF_STATUS_COLLISION = 2,       // Collission detected
    DF_STATUS_TIMEOUT = 3,         // Timeout in communication.
    DF_STATUS_NO_ROOM = 4,         // A buffer is not big enough.
    DF_STATUS_INTERNAL_ERROR = 5,  // Internal error in the code. Should not happen ;-)
    DF_STATUS_INVALID = 6,         // Invalid argument.
    DF_STATUS_CRC_WRONG = 7,       // The CRC_A does not match

    COMMAND_NOT_FOUND = 8,          // (910B) - Status used only in Read_Sig.
    COMMAND_FORMAT_ERROR = 9,       // (910C) - Status used only in Read_Sig.
    ILLEGAL_COMMAND_CODE = 10,      // (911C) Command code not supported.
    INTEGRITY_ERROR = 11,           // (911E) CRC or MAC does not match data. Padding bytes not valid.
    NO_SUCH_KEY = 12,               // (9140) Invalid key number specified.
    LENGTH_ERROR = 13,              // (6700, 917E) Length of command string invalid.
    PERMISSION_DENIED = 14,         // (919D) Current configuration / status does not allow the requested command.
    PARAMETER_ERROR = 15,           // (919E) Value of the parameter(s) invalid.
    AUTHENTICATION_DELAY = 16,      // (91AD) Currently not allowed to authenticate. Keep trying until full delay is spent.
    AUTHENTICATION_ERROR = 17,      // (91AE) Current authentication status does not allow the requested command.
    ADDITIONAL_FRAME = 18,          // (91AF) Additionaldata frame is expected to be sent.
    BOUNDARY_ERROR = 19,            // (91BE) Attempt to read/write data from/to beyond the file's/record's limits. Attempt to exceed the limits of a value file.
    COMMAND_ABORTED = 20,           // (91CA) Previous Command was not fully completed. Not all Frames were requested or provided by the PCD.
    MEMORY_ERROR = 21,              // (6581, 91EE) Failure when reading or writing to non-volatile memory.
    FILE_NOT_FOUND = 22,            // (91F0) Specified file number does not exist.
    SECURITY_NOT_SATISFIED = 23,    // (6982) Security status not satisfied.
    CONDITIONS_NOT_SATISFIED = 24,  // (6985) Conditions of use not satisfied.
    FILE_OR_APP_NOT_FOUND = 25,     // (6A82) File or application not found.
    INCORRECT_PARAMS = 26,          // (6A86) Incorrect parameters P1-P2.
    INCORRECT_LC = 27,              // (6A87) Lc inconsistent with parameters P1-P2.
    CLA_NOT_SUPPORTED = 28,         // (6E00) CLA not supported

    DF_WRONG_RESPONSE_LEN = 29,
    DF_WRONG_RESPONSE_CMAC = 30,
    DF_WRONG_RNDA = 31,
    DF_CMD_CTR_OVERFLOW = 32,
    DF_UNKNOWN_ERROR = 33,
    DF_SDM_NOT_IMPLEMENTED_IN_LIB = 34,
    DUPLICATE_ERROR = 35,      // (91DE) on application or file creation: file or application is existing
    OUT_OF_EEPROM_ERROR = 36,  // (910E) on application or file creation: no more memory available

    DF_STATUS_MIFARE_NACK = 0xff  // A MIFARE PICC responded with NAK.
  };

  enum DF_CommMode : byte {
    DF_COMMMODE_PLAIN = 0x00,  // 0b00, but can be 0b10 (0x02) as well
    DF_COMMMODE_MAC = 0x01,    // 0b01
    DF_COMMMODE_FULL = 0x03    // 0b11
  };

  // Limitations on PN532 readers
  const uint8_t MAX_BUFFER_SIZE = 125;  // the internal buffer is 128 - 3 for status bytes
  const uint8_t PLAIN_MAX_WRITE_LENGTH = 96;

  /////////////////////////////////////////////////////////////////////////////////////
  //
  // Application Handling
  //
  /////////////////////////////////////////////////////////////////////////////////////

  // Application handling
  DF_StatusCode DF_Plain_SelectApplication(byte* aid);
  DF_StatusCode DF_Plain_CreateApplication(byte* aid, byte keySettings, byte appSettings);
  DF_StatusCode DF_Plain_CreateApplicationDefaultAes(byte* aid);

  /////////////////////////////////////////////////////////////////////////////////////
  //
  // File Handling
  //
  /////////////////////////////////////////////////////////////////////////////////////

  DF_StatusCode DF_Plain_CreateStandardDataFile(byte fileNo, DF_CommMode commMode, byte accessRightsRwCar, byte accessRightsRW, byte length);
  DF_StatusCode DF_Plain_CreateStandardFileDefault32(byte fileNo, DF_CommMode commMode);
  DF_StatusCode DF_Plain_CreateStandardFileDefaultSized(byte fileNo, byte fileSize, DF_CommMode commMode);
  DF_StatusCode DF_Plain_CreateStandardFileDefaultFreeAccessSized(byte fileNo, byte fileSize, DF_CommMode commMode);

  DF_StatusCode DF_Plain_ReadData_Simple(byte fileNo, uint16_t length, byte offset, byte* backReadData, uint16_t* backReadLen);
  DF_StatusCode DF_Plain_GetMoreData_native(byte* backRespData, byte* backRespLen);

  DF_StatusCode DF_Plain_WriteData_Simple(byte fileNo, byte length, byte offset, byte* sendData);

  DF_StatusCode DF_Plain_GetFileSettings(byte fileNo, byte* backRespData, byte* backRespLen);
  void DF_FileSettingsDebugPrint();
  void DF_StatusCodeDebugPrint(DF_StatusCode statusCode);

  /////////////////////////////////////////////////////////////////////////////////////
  //
  // PICC Handling
  //
  /////////////////////////////////////////////////////////////////////////////////////

  DF_StatusCode DF_Plain_GetFreeMemory(byte* backData, byte* backLen);

  // Writes to backRespData 28 or 29 bytes according to tables 54, 56 and 58 from NT4H2421Gx (NTAG 424 DNA) datasheet:
  // VendorID, HWType, HWSubType, HWMajorVersion, HWMinorVersion, HWStorageSize, HWProtocol,
  // VendorID, SWType, SWSubType, SWMajorVersion, SWMinorVersion, SWStorageSize, SWProtocol,
  // UID(7), BatchNo(4), BatchNo/FabKey, FabKey/CWProd, YearProd, [FabKeyID]
  // Note: arguments in brackets are optional; SW1 and SW2 are not included in backRespData
  DF_StatusCode DF_Plain_GetVersion(byte* backRespData, byte* backRespLen);

  // helper methods
  void printHex(byte* buffer, uint16_t bufferSize);
  void convertLargeInt2Uint8_t4Lsb(int& input, uint8_t* output);
  void convertInt2Uint8_t(int& input, uint8_t* output);
  int convertUint8_t3_2IntLsb(byte* input);
  int convertUint8_t4_2IntLsb(byte* input);
  void convertIntTo3BytesLsb(int input, byte* output);
  void revertAid(byte* aid, byte* revertAid);
  void hexCharacterStringToBytes(byte* byteArray, const char* hexString);
  byte nibble(char c);
  void dumpByteArray(const byte* byteArray, const byte arraySize);

private:

  Adafruit_PN532* nfcLib;

  // data is retrieved by getFileSettings
  // see https://www.nxp.com/docs/en/data-sheet/MF2DLHX0.pdf DESFire Light Features & Hints, pages 75ff
  bool fileSettingsIsValid = false;
  byte fileSettingsFileNo;
  byte fileSettingsFileType;     // 0x00 - 0x04, 0x05 Transaction file not included
  byte fileSettingsFileOptions;  // e.g. comm modes (bits 0 & 1) and SDM options (bits 2-7)
  bool isFilePlainCommunicationMode = false;
  bool isFileMacCommunicationMode = false;
  bool isFileFullCommunicationMode = false;
  byte fileSettingsRwCarAccessRights;
  byte fileSettingsRWAccessRights;
  uint16_t fileSettingsFileSize;                    // data files only
  uint16_t fileSettingsLowerLimit;                  // value files only
  uint16_t fileSettingsUpperLimit;                  // value files only
  uint16_t fileSettingsLimitedCreditValue;          // value files only
  bool fileSettingsIsLimitedCreditEnabled = false;  // value files only
  bool fileSettingsIsFreeAccessToGetValue = false;  // value files only
  uint16_t fileSettingsRecordSize;                  // record files only
  uint16_t fileSettingsMaxNoOfRecs;                 // record files only
  uint16_t fileSettingsCurrentNoOfRecs;             // record files only

protected:

  /////////////////////////////////////////////////////////////////////////////////////
  //
  // Protected functions
  //
  /////////////////////////////////////////////////////////////////////////////////////

  DF_StatusCode DF_BasicTransceive(byte* sendData, byte sendLen, byte* backData, byte* backLen);
  DF_StatusCode DF_InterpretErrorCode(byte* SW1_2);

  DF_StatusCode DF_Plain_CreateDataFile_native(byte CMD, byte fileNo, DF_CommMode commMode, byte accessRightsRwCar, byte accessRightsRW, byte length);
  DF_StatusCode DF_Plain_GetVersion_native(byte Cmd, byte expectedSV2, byte* backRespData, byte* backRespLen);

  bool DF_GetFileSettingsAnalyzer(byte fileNo, byte* resData, uint8_t resLen);
};

#endif