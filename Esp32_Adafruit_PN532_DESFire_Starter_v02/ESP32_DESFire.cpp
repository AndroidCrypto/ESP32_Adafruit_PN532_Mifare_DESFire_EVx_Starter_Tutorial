
#include "ESP32_DESFire.h"
#include <Adafruit_PN532.h>

/////////////////////////////////////////////////////////////////////////////////////
//
// Basic functions for communicating with Mifare DESFire EVx cards
//
/////////////////////////////////////////////////////////////////////////////////////

ESP32_DESFire::ESP32_DESFire(Adafruit_PN532* nfc) {
  nfcLib = nfc;
}

/////////////////////////////////////////////////////////////////////////////////////
//
// Application management
//
/////////////////////////////////////////////////////////////////////////////////////

ESP32_DESFire::DF_StatusCode ESP32_DESFire::DF_Plain_SelectApplication(byte* aid) {

  uint8_t i;
  byte sendData[9];

  sendData[0] = 0x90;
  sendData[1] = DESFIRE_SELECT_APPLICATION;
  sendData[2] = 0x00;
  sendData[3] = 0x00;
  sendData[4] = 0x03;

  // copy the AID in parameter
  for (i = 0; i < 3; i++) {
    sendData[5 + i] = aid[i]; /* 3 byte AID */
  }
  sendData[8] = 0x00;

  byte backData[61];
  byte backLen = 61;

  DF_StatusCode statusCode;

  statusCode = DF_BasicTransceive(sendData, sizeof(sendData), backData, &backLen);

  if (statusCode != DF_STATUS_OK)
    return (DF_StatusCode)statusCode;

  if (backLen != 2)
    return DF_STATUS_ERROR;

  if (backData[backLen - 2] != 0x91 || backData[backLen - 1] != 0x00)
    return DF_InterpretErrorCode(&backData[backLen - 2]);

  return DF_STATUS_OK;
}

ESP32_DESFire::DF_StatusCode ESP32_DESFire::DF_Plain_CreateApplication(byte* aid, byte keySettings, byte appSettings) {
  uint8_t i;
  int8_t _aid[3];  // DESFire aid
  int8_t _aidLen;  // aid len = 3
  // Hang on to the aid data
  memcpy(_aid, aid, 3);
  _aidLen = 3;

  byte sendData[11];

  sendData[0] = 0x90;                        // CLA
  sendData[1] = DESFIRE_CREATE_APPLICATION;  // CMD DESFIRE_SELECT_APPLICATION (0xCA)
  sendData[2] = 0x00;                        // P1
  sendData[3] = 0x00;                        // P2
  sendData[4] = 0x05;                        // Lc

  // copy the AID in parameter
  for (i = 0; i < _aidLen; i++) {
    sendData[5 + i] = _aid[i];  // 3 byte AID
  }
  sendData[8] = keySettings;
  sendData[9] = appSettings;
  sendData[10] = 0x00;  // Le

  byte backData[61];
  byte backLen = 61;

  bool success;
  DF_StatusCode statusCode;

  statusCode = DF_BasicTransceive(sendData, sizeof(sendData), backData, &backLen);

  if (statusCode != DF_STATUS_OK)
    return (DF_StatusCode)statusCode;

  if (backLen < 2)
    return DF_WRONG_RESPONSE_LEN;

  if (backData[backLen - 2] != 0x91 || backData[backLen - 1] != 0x00)
    return DF_InterpretErrorCode(&backData[backLen - 2]);

  return DF_STATUS_OK;
}

// creates an application with 5 AES keys and default app settings
ESP32_DESFire::DF_StatusCode ESP32_DESFire::DF_Plain_CreateApplicationDefaultAes(byte* aid) {
  /**
   * for explanations on Master Key Settings see M075031_desfire.pdf page 35:
   * left '0' = Application master key authentication is necessary to change any key (default)
   * right 'f' = bits 3..0
   * bit 3: 1: this configuration is changeable if authenticated with the application master key (default setting)
   * bit 2: 1: CreateFile / DeleteFile is permitted also without application master key authentication (default setting)
   * bit 1: 1: GetFileIDs, GetFileSettings and GetKeySettings commands succeed independently of a preceding application master key authentication (default setting)
   * bit 0: 1: Application master key is changeable (authentication with the current application master key necessary, default setting)
  */
  byte keySettings = 0x0F;
  byte appSettings = 0x85;  // first nibble '8' = AES, second nibble '5' = 5 keys
  return DF_Plain_CreateApplication(aid, keySettings, appSettings);
};

/////////////////////////////////////////////////////////////////////////////////////
//
// Data File management
//
/////////////////////////////////////////////////////////////////////////////////////

ESP32_DESFire::DF_StatusCode ESP32_DESFire::DF_Plain_ReadData_Simple(byte fileNo, uint16_t length, byte offset, byte* backReadData, uint16_t* backReadLen) {
  ESP32_DESFire::DF_StatusCode df_statusCode;

  byte sendData[13];

  sendData[0] = 0x90;                    // CLA
  sendData[1] = DESFIRE_READ_DATA_FILE;  // CMD 0xAD
  sendData[2] = 0x00;                    // P1
  sendData[3] = 0x00;                    // P2
  sendData[4] = 0x07;                    // LC
  sendData[5] = fileNo;                  // FileNo
  sendData[6] = offset;                  // Offset
  sendData[7] = 0x00;                    // (Offset)
  sendData[8] = 0x00;                    // (Offset)
  sendData[9] = length;                  // Length
  sendData[10] = 0x00;                   // (Length)
  sendData[11] = 0x00;                   // (Length)
  sendData[12] = 0x00;                   // Le

  byte backData[MAX_BUFFER_SIZE];
  byte backLen = MAX_BUFFER_SIZE;

  DF_StatusCode statusCode;
  statusCode = DF_BasicTransceive(sendData, sizeof(sendData), backData, &backLen);

  if (statusCode != DF_STATUS_OK) {
    return (DF_StatusCode)statusCode;
  }

  if (backLen < 2) return DF_WRONG_RESPONSE_LEN;  // something gone wrong

  if (backData[backLen - 2] == 0x91 && backData[backLen - 1] != DF_STATUS_OK) {
    *backReadLen = 0;
    return DF_InterpretErrorCode(&backData[backLen - 2]);
  }

  // at this point our data is complete
  if ((backLen - 2) != length) {
    return DF_WRONG_RESPONSE_LEN;
  }

  if (*backReadLen < length) {
    return DF_STATUS_NO_ROOM;
  }

  if (backLen > 0)
    memcpy(backReadData, backData, length);
  *backReadLen = length;

  return DF_STATUS_OK;
}

void ESP32_DESFire::hexCharacterStringToBytes(byte* byteArray, const char* hexString) {
  bool oddLength = strlen(hexString) & 1;

  byte currentByte = 0;
  byte byteIndex = 0;

  for (byte charIndex = 0; charIndex < strlen(hexString); charIndex++) {
    bool oddCharIndex = charIndex & 1;

    if (oddLength) {
      // If the length is odd
      if (oddCharIndex) {
        // odd characters go in high nibble
        currentByte = nibble(hexString[charIndex]) << 4;
      } else {
        // Even characters go into low nibble
        currentByte |= nibble(hexString[charIndex]);
        byteArray[byteIndex++] = currentByte;
        currentByte = 0;
      }
    } else {
      // If the length is even
      if (!oddCharIndex) {
        // Odd characters go into the high nibble
        currentByte = nibble(hexString[charIndex]) << 4;
      } else {
        // Odd characters go into low nibble
        currentByte |= nibble(hexString[charIndex]);
        byteArray[byteIndex++] = currentByte;
        currentByte = 0;
      }
    }
  }
}

byte ESP32_DESFire::nibble(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';

  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;

  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;

  return 0;  // Not a valid hexadecimal character
}

void ESP32_DESFire::dumpByteArray(const byte* byteArray, const byte arraySize) {

  for (int i = 0; i < arraySize; i++) {
    Serial.print("0x");
    if (byteArray[i] < 0x10)
      Serial.print("0");
    Serial.print(byteArray[i], HEX);
    Serial.print(", ");
  }
  Serial.println();
}

ESP32_DESFire::DF_StatusCode ESP32_DESFire::DF_Plain_WriteData_Simple(byte fileNo, byte length, byte offset, byte* sendData) {

  byte* sendData2 = new byte[length + 13];

  sendData2[0] = 0x90;                     // CLA
  sendData2[1] = DESFIRE_WRITE_DATA_FILE;  // CMD 0x8D
  sendData2[2] = 0x00;                     // P1
  sendData2[3] = 0x00;                     // P2
  sendData2[4] = 7 + length;               // Lc
  sendData2[5] = fileNo;                   // FileNo
  sendData2[6] = offset;                   // Offset
  sendData2[7] = 0x00;                     // (Offset)
  sendData2[8] = 0x00;                     // (Offset)
  sendData2[9] = length;                   // Length
  sendData2[10] = 0x00;                    // (Length)
  sendData2[11] = 0x00;                    // (Length)
  memcpy(&sendData2[12], sendData, length);
  sendData2[length + 12] = 0x00;  // Le

  byte backData[64];
  byte backLen = 64;

  ESP32_DESFire::DF_StatusCode statusCode;
  statusCode = DF_BasicTransceive(sendData2, length + 13, backData, &backLen);

  delete[] sendData2;

  if (statusCode != DF_STATUS_OK)
    return (DF_StatusCode)statusCode;

  if (backData[backLen - 2] != 0x91 || backData[backLen - 1] != 0x00)
    return DF_InterpretErrorCode(&backData[backLen - 2]);

  if (backLen != 2)
    return DF_WRONG_RESPONSE_LEN;

  return DF_STATUS_OK;
}

// Note: the maximal length is 255 bytes as no int to LSB conversion is done
ESP32_DESFire::DF_StatusCode ESP32_DESFire::DF_Plain_CreateStandardDataFile(byte fileNo, DF_CommMode commMode, byte accessRightsRwCar, byte accessRightsRW, byte length) {
  return DF_Plain_CreateDataFile_native(DESFIRE_CREATE_STANDARD_DATA_FILE, fileNo, commMode, accessRightsRwCar, accessRightsRW, length);
}

ESP32_DESFire::DF_StatusCode ESP32_DESFire::DF_Plain_CreateStandardFileDefault32(byte fileNo, DF_CommMode commMode) {
  byte accessRightsRwCar = 0x12;  // key 1 has R&W access rights, key 2 has CAR rights
  byte accessRightsRW = 0x34;     // key 3 has R, key 4 has W access rights
  byte length = 0x20;             // 32 bytes
  return DF_Plain_CreateStandardDataFile(fileNo, commMode, accessRightsRwCar, accessRightsRW, length);
}

// This gives a Standard File with free accessible data
ESP32_DESFire::DF_StatusCode ESP32_DESFire::DF_Plain_CreateStandardFileDefaultSized(byte fileNo, byte fileSize, DF_CommMode commMode) {
  byte accessRightsRwCar = 0x12;  // key 1 has R&W access rights, key 2 has CAR rights
  byte accessRightsRW = 0x34;     // key 3 has R, key 4 has W access rights
  return DF_Plain_CreateStandardDataFile(fileNo, commMode, accessRightsRwCar, accessRightsRW, fileSize);
}

ESP32_DESFire::DF_StatusCode ESP32_DESFire::DF_Plain_CreateStandardFileDefaultFreeAccessSized(byte fileNo, byte fileSize, DF_CommMode commMode) {
  byte accessRightsRwCar = 0xEE;  // key 1 has R&W access rights, key 2 has CAR rights
  byte accessRightsRW = 0xEE;     // key 3 has R, key 4 has W access rights
  return DF_Plain_CreateStandardDataFile(fileNo, commMode, accessRightsRwCar, accessRightsRW, fileSize);
}

// Note: the maximal length is 255 bytes as no int to LSB conversion is implemented
ESP32_DESFire::DF_StatusCode ESP32_DESFire::DF_Plain_CreateDataFile_native(byte Cmd, byte fileNo, DF_CommMode commMode, byte accessRightsRwCar, byte accessRightsRW, byte length) {
  byte sendData[13];

  sendData[0] = 0x90;               // CLA
  sendData[1] = Cmd;                // Standard Data: 0xCD or Backup Data: 0xCB
  sendData[2] = 0x00;               // P1
  sendData[3] = 0x00;               // P2
  sendData[4] = 0x07;               // LC
  sendData[5] = fileNo;             // FileNo
  sendData[6] = commMode;           // CommunicationMode 00 = Plain, 01 = MAC, 03 = Full
  sendData[7] = accessRightsRwCar;  // Access Rights for R&W and CAR keys
  sendData[8] = accessRightsRW;     // Access Rights for R and W keys
  sendData[9] = length;             // Length LSB
  sendData[10] = 0x00;              // (Length) ### needs an int -> LSB conversion
  sendData[11] = 0x00;              // (Length)
  sendData[12] = 0x00;              // Le

  byte backData[61];
  byte backLen = 61;

  DF_StatusCode statusCode;
  statusCode = DF_BasicTransceive(sendData, sizeof(sendData), backData, &backLen);

  if (statusCode != DF_STATUS_OK)
    return (DF_StatusCode)statusCode;

  if (backData[backLen - 2] != 0x91 || backData[backLen - 1] != 0x00)
    return DF_InterpretErrorCode(&backData[backLen - 2]);

  if (backLen > length + 2)
    return DF_WRONG_RESPONSE_LEN;

  return DF_STATUS_OK;
}

ESP32_DESFire::DF_StatusCode ESP32_DESFire::DF_Plain_GetFileSettings(byte fileNo, byte* backRespData, byte* backRespLen) {
  byte sendData[7];

  sendData[0] = 0x90;                       // CLA
  sendData[1] = DESFIRE_GET_FILE_SETTINGS;  // CMD 0xF5
  sendData[2] = 0x00;                       // P1
  sendData[3] = 0x00;                       // P2
  sendData[4] = 0x01;                       // Lc
  sendData[5] = fileNo;                     // FileNo
  sendData[6] = 0x00;                       // Le

  byte backData[61];
  byte backLen = 61;

  DF_StatusCode statusCode;
  statusCode = DF_BasicTransceive(sendData, sizeof(sendData), backData, &backLen);

  if (statusCode != DF_STATUS_OK)
    return (DF_StatusCode)statusCode;

  if (backData[backLen - 2] != 0x91 || backData[backLen - 1] != 0x00)
    return DF_InterpretErrorCode(&backData[backLen - 2]);

  if (backLen < 9 || backLen > 36)
    return DF_WRONG_RESPONSE_LEN;

  if (*backRespLen < backLen - 2)
    return DF_STATUS_NO_ROOM;

  memcpy(backRespData, backData, backLen - 2);
  *backRespLen = backLen - 2;

  fileSettingsIsValid = false;  // invalidate the old data
  DF_GetFileSettingsAnalyzer(fileNo, backData, backLen - 10);

  return DF_STATUS_OK;
}

void ESP32_DESFire::DF_FileSettingsDebugPrint() {
  Serial.printf("File Settings for file %02x\n", fileSettingsFileNo);
  if (!fileSettingsIsValid) {
    Serial.println("The FileSettings for this file are INVALID, aborting");
    return;
  }
  Serial.printf("File Type         : %02x ", fileSettingsFileType);
  if (fileSettingsFileType == 0x00) {
    Serial.println("(Standard Data File)");
  } else if (fileSettingsFileType == 0x01) {
    Serial.println("(Backup Data File)");
  } else if (fileSettingsFileType == 0x02) {
    Serial.println("(Value File)");
  } else if (fileSettingsFileType == 0x03) {
    Serial.println("(Linear Record File)");
  } else if (fileSettingsFileType == 0x04) {
    Serial.println("(Cyclic Record File)");
  } else if (fileSettingsFileType == 0x05) {
    Serial.println("(Transaction MAC File)");
  } else {
    Serial.println("(Unknown File Type)");
  }
  Serial.printf("File Options      : %02X\n", fileSettingsFileOptions);
  if (isFilePlainCommunicationMode) {
    Serial.println("File Comm Mode    : PLAIN");
  } else if (isFileMacCommunicationMode) {
    Serial.println("File Comm Mode    : MAC");
  } else {
    Serial.println("File Comm Mode    : FULL encrypted");
  }
  Serial.printf("File RW/CAR AccRg : %02X\n", fileSettingsRwCarAccessRights);
  Serial.printf("File R/W    AccRg : %02X\n", fileSettingsRWAccessRights);
  if ((fileSettingsFileType == 0x00) || (fileSettingsFileType == 0x01)) {
    // data file
    Serial.printf("File Size         : %d\n", fileSettingsFileSize);
  } else if (fileSettingsFileType == 0x02) {
    // value file
    Serial.printf("File Size         : %d\n", fileSettingsFileSize);
    Serial.printf("Lower Limit       : %d\n", fileSettingsLowerLimit);
    Serial.printf("Upper Limit       : %d\n", fileSettingsUpperLimit);
    Serial.printf("Limited Credit    : %d\n", fileSettingsLimitedCreditValue);
    if (fileSettingsIsLimitedCreditEnabled) {
      Serial.println("Limtd Cred Enabld : YES");
    } else {
      Serial.println("Limtd Cred Enabld : NO");
    }
    if (fileSettingsIsFreeAccessToGetValue) {
      Serial.println("FreeAccess GetVal : YES");
    } else {
      Serial.println("FreeAccess GetVal : NO");
    }
  } else if ((fileSettingsFileType == 0x03) || (fileSettingsFileType == 0x04)) {
    // record file
    Serial.printf("File Record Size  : %d\n", fileSettingsRecordSize);
    Serial.printf("Max No of Records : %d\n", fileSettingsMaxNoOfRecs);
    Serial.printf("Cur No of Records : %d\n", fileSettingsCurrentNoOfRecs);
  }
}

// This does include just a subset of possible error codes
void ESP32_DESFire::DF_StatusCodeDebugPrint(DF_StatusCode statusCode) {
  switch (statusCode) {
    case DF_STATUS_OK: Serial.println("SUCCESS"); break;
    case PERMISSION_DENIED: Serial.println("PERMISSION_DENIED ERROR"); break;
    case FILE_NOT_FOUND: Serial.println("FILE/APP NOT FOUND ERROR"); break;
    case DUPLICATE_ERROR: Serial.println("DUPLICATE ERROR"); break;
    default: Serial.println("FAIL (not categorized)"); break;
  }
}

// This is a GetFileSettings response analyzer. Please call it on DF_STATUS_OK result from
// GetFileSettings only.
bool ESP32_DESFire::DF_GetFileSettingsAnalyzer(byte fileNo, byte* resData, uint8_t resLen) {

  // get the data if the response is long enough
  if (resLen >= 7) {
    fileSettingsFileNo = fileNo;
    fileSettingsFileType = resData[0];
    fileSettingsFileOptions = resData[1];
    // determine the communication mode on bits 0 and 1
    // see NTAG 424 DNA data sheet, page 70 and 13
    /*
Communication mode - Bit Representation
CommMode.Plain     - 00b // X0b means 0 or 1 for X
CommMode.Plain     - 10b // X0b means 0 or 1 for X
CommMode.MAC       - 01b 
CommMode.Full      - 11b
*/
    // reset
    isFilePlainCommunicationMode = false;
    isFileMacCommunicationMode = false;
    isFileFullCommunicationMode = false;
    bool isBit0 = bitRead(fileSettingsFileOptions, 0);
    bool isBit1 = bitRead(fileSettingsFileOptions, 1);
    if (!isBit0) {
      isFilePlainCommunicationMode = true;
    } else {
      if (!isBit1) {
        isFileMacCommunicationMode = true;
      } else {
        isFileFullCommunicationMode = true;
      }
    }
    fileSettingsRwCarAccessRights = resData[2];
    fileSettingsRWAccessRights = resData[3];
    if ((fileSettingsFileType == 0x00) || (fileSettingsFileType == 0x01)) {
      // data file
      fileSettingsFileSize = resData[4] + (resData[5] * 256) + (resData[6] * 65536);
      fileSettingsIsValid = true;
    } else if (fileSettingsFileType == 0x02) {
      // value file
      fileSettingsLowerLimit = resData[4] + (resData[5] * 256) + (resData[6] * 65536) + (resData[7] * 16777216);
      fileSettingsUpperLimit = resData[8] + (resData[9] * 256) + (resData[10] * 65536) + (resData[11] * 16777216);
      fileSettingsLimitedCreditValue = resData[12] + (resData[13] * 256) + (resData[14] * 65536) + (resData[15] * 16777216);
      fileSettingsIsLimitedCreditEnabled = bitRead(resData[16], 0);
      fileSettingsIsFreeAccessToGetValue = bitRead(resData[16], 1);
      fileSettingsIsValid = true;
    } else if ((fileSettingsFileType == 0x03) || (fileSettingsFileType == 0x04)) {
      if (resLen >= 13) {
        fileSettingsRecordSize = resData[4] + (resData[5] * 256) + (resData[6] * 65536);
        fileSettingsMaxNoOfRecs = resData[7] + (resData[8] * 256) + (resData[9] * 65536);
        fileSettingsCurrentNoOfRecs = resData[10] + (resData[11] * 256) + (resData[12] * 65536);
        fileSettingsIsValid = true;
      } else {
        fileSettingsIsValid = false;
      }
    }
  } else {
    fileSettingsIsValid = false;
  }
  return fileSettingsIsValid;
}

/////////////////////////////////////////////////////////////////////////////////////
//
// PICC management
//
/////////////////////////////////////////////////////////////////////////////////////

// Writes to backRespData 28 or 29 bytes according to tables 54, 56, 58 from NT4H2421Gx (NTAG 424 DNA) datasheet:
// VendorID, HWType, HWSubType, HWMajorVersion, HWMinorVersion, HWStorageSize, HWProtocol,
// VendorID, SWType, SWSubType, SWMajorVersion, SWMinorVersion, SWStorageSize, SWProtocol,
// UID(7), BatchNo(4), BatchNo/FabKey, FabKey/CWProd, YearProd, [FabKeyID]
// Note: arguments in brackets are optional; SW1 and SW2 are not included in backRespData

ESP32_DESFire::DF_StatusCode ESP32_DESFire::DF_Plain_GetVersion(byte* backRespData, byte* backRespLen) {
  byte backData[29];
  byte backLen;

  ESP32_DESFire::DF_StatusCode statusCode;
  statusCode = DF_Plain_GetVersion_native(DESFIRE_GET_VERSION, DESFIRE_GET_MORE_DATA, backData, &backLen);

  if (statusCode != DF_STATUS_OK)
    return statusCode;

  if (backLen != 7)
    return DF_WRONG_RESPONSE_LEN;

  statusCode = DF_Plain_GetVersion_native(DESFIRE_GET_MORE_DATA, DESFIRE_GET_MORE_DATA, &backData[7], &backLen);

  if (statusCode != DF_STATUS_OK)
    return statusCode;

  if (backLen != 7)
    return DF_WRONG_RESPONSE_LEN;

  statusCode = DF_Plain_GetVersion_native(DESFIRE_GET_MORE_DATA, DESFIRE_SV2_OK, &backData[14], &backLen);

  if (statusCode != DF_STATUS_OK)
    return statusCode;

  if (backLen != 14 && backLen != 15)
    return DF_WRONG_RESPONSE_LEN;

  if (*backRespLen < backLen + 14)
    return DF_STATUS_NO_ROOM;

  memcpy(backRespData, backData, backLen + 14);
  *backRespLen = backLen + 14;

  return DF_STATUS_OK;
}

ESP32_DESFire::DF_StatusCode ESP32_DESFire::DF_Plain_GetVersion_native(byte Cmd, byte expectedSV2, byte* backRespData, byte* backRespLen) {
  byte sendData[5];

  sendData[0] = 0x90;  // CLA
  sendData[1] = Cmd;   // CMD
  sendData[2] = 0x00;  // P1
  sendData[3] = 0x00;  // P2
  sendData[4] = 0x00;  // Le

  byte backData[61];
  byte backLen = 61;
  DF_StatusCode statusCode;

  statusCode = DF_BasicTransceive(sendData, sizeof(sendData), backData, &backLen);

  if (statusCode != DF_STATUS_OK)
    return (DF_StatusCode)statusCode;

  if (backData[backLen - 2] != 0x91 || backData[backLen - 1] != expectedSV2)
    return DF_InterpretErrorCode(&backData[backLen - 2]);

  if (backLen != 9 && backLen != 16 && backLen != 17 && backLen != 24 && backLen != 25)
    return DF_WRONG_RESPONSE_LEN;

  memcpy(backRespData, backData, backLen - 2);
  *backRespLen = backLen - 2;

  return DF_STATUS_OK;
}

ESP32_DESFire::DF_StatusCode ESP32_DESFire::DF_Plain_GetFreeMemory(byte* backRespData, byte* backRespLen) {
  byte sendData[5];

  sendData[0] = 0x90;                     // CLA
  sendData[1] = DESFIRE_GET_FREE_MEMORY;  // CMD 0x6E
  sendData[2] = 0x00;                     // P1
  sendData[3] = 0x00;                     // P2
  sendData[4] = 0x00;                     // Le

  byte backData[61];
  byte backLen = 61;
  DF_StatusCode statusCode;
  statusCode = DF_BasicTransceive(sendData, sizeof(sendData), backData, &backLen);

  if (statusCode != DF_STATUS_OK)
    return statusCode;

  if (backLen < 2)
    return DF_WRONG_RESPONSE_LEN;

  if (backData[backLen - 2] != 0x91 || backData[backLen - 1] != 0x00)
    return DF_InterpretErrorCode(&backData[backLen - 2]);

  if (backLen != 5)
    return DF_WRONG_RESPONSE_LEN;

  memcpy(backRespData, backData, backLen - 2);
  *backRespLen = backLen - 2;

  return DF_STATUS_OK;
}

// This is returning the full response, not just the data without status codes
ESP32_DESFire::DF_StatusCode ESP32_DESFire::DF_Plain_GetMoreData_native(byte* backRespData, byte* backRespLen) {
  byte sendData[5];

  sendData[0] = 0x90;                   // CLA
  sendData[1] = DESFIRE_GET_MORE_DATA;  // CMD 0xAF
  sendData[2] = 0x00;                   // P1
  sendData[3] = 0x00;                   // P2
  sendData[4] = 0x00;                   // Le

  byte backData[61];
  byte backLen = 61;
  DF_StatusCode statusCode;

  statusCode = DF_BasicTransceive(sendData, sizeof(sendData), backData, &backLen);

  if (statusCode != DF_STATUS_OK)
    return (DF_StatusCode)statusCode;

  memcpy(backRespData, backData, backLen);  // return the complete response
  *backRespLen = backLen;

  if (backData[backLen - 2] == 0x91 && backData[backLen - 1] == 0xAF) {
    Serial.println("Get_More_Data Returning ADDITIONAL_FRAME");
    return ADDITIONAL_FRAME;
  } else if (backData[backLen - 2] == 0x91 && backData[backLen - 1] == 0x00) {
    Serial.println("Get_More_Data Returning DF_STATUS_OK");
    return DF_STATUS_OK;
  } else {
    return DF_UNKNOWN_ERROR;
  }
}

void ESP32_DESFire::convertLargeInt2Uint8_t4Lsb(int& input, uint8_t* output) {
  int max2Byte = 65536;
  int highInt = input / max2Byte;
  int lowInt = input % max2Byte;
  uint8_t highUint8[2];
  convertInt2Uint8_t(highInt, highUint8);
  uint8_t lowUint8[2];
  convertInt2Uint8_t(lowInt, lowUint8);
  uint8_t convertedInt[4];
  convertedInt[3] = highUint8[0];
  convertedInt[2] = highUint8[1];
  convertedInt[1] = lowUint8[0];
  convertedInt[0] = lowUint8[1];
  int i;
  for (i = 0; i < 4; i++) {
    output[i] = convertedInt[i];
  }
}

// maximum int is 65535
void ESP32_DESFire::convertInt2Uint8_t(int& input, uint8_t* output) {
  byte high = highByte(input);
  byte low = lowByte(input);
  //uint8_t uint8_22[2];
  output[0] = high;
  output[1] = low;
}

int ESP32_DESFire::convertUint8_t3_2IntLsb(byte* input) {
  int outputValue = 0;
  outputValue += input[0];
  outputValue += (input[1] * 256);
  outputValue += (input[2] * 65536);
  //output = outputValue;
  return outputValue;
}

int ESP32_DESFire::convertUint8_t4_2IntLsb(byte* input) {
  int outputValue = 0;
  outputValue += input[0];
  outputValue += (input[1] * 256);
  outputValue += (input[2] * 65536);
  outputValue += (input[3] * 16777216);
  return outputValue;
}

void ESP32_DESFire::convertIntTo3BytesLsb(int input, byte* output) {
  byte* out = new byte[3];
  if (input > 16777215) {
    memset(output, 0, 3);
    //memcpy(output, out, 3);
    return;
  }
  out[0] = input & 0xff;
  out[1] = (input >> 8) & 0xff;
  out[2] = (input >> 16) & 0xff;
  memcpy(output, out, 3);
}

void ESP32_DESFire::revertAid(byte* aid, byte* revertAid) {
  byte aidRevert[3];
  aidRevert[0] = aid[2];
  aidRevert[1] = aid[1];
  aidRevert[2] = aid[0];
  memcpy(revertAid, aidRevert, 3);
}

/////////////////////////////////////////////////////////////////////////////////////
//
// Protected functions
//
/////////////////////////////////////////////////////////////////////////////////////

ESP32_DESFire::DF_StatusCode ESP32_DESFire::DF_BasicTransceive(byte* sendData, byte sendLen, byte* backData, byte* backLen) {
  ESP32_DESFire::DF_StatusCode result;
  bool success;
  DF_StatusCode statusCode;
  byte bLen = 255;  // ### changed from 61 to 255
  if (COMM_DEBUG_PRINT) {
    Serial.printf("Send length %d\n", sendLen);
    printHex(sendData, sendLen);
    Serial.println("");
  }
  success = nfcLib->inDataExchange(sendData, sendLen, backData, &bLen);
  if (COMM_DEBUG_PRINT) {
    Serial.printf("Recv length %d\n", bLen);
    printHex(backData, bLen);
    Serial.println("");
    *backLen = bLen;
    return DF_STATUS_OK;
  } else {
    *backLen = 0;
    return DF_STATUS_ERROR;
  }
}

ESP32_DESFire::DF_StatusCode ESP32_DESFire::DF_InterpretErrorCode(byte* SW1_2) {
  uint16_t SW = SW1_2[0] << 8 | SW1_2[1];
  switch (SW) {
    case 0x6581:
      return ESP32_DESFire::MEMORY_ERROR;
    case 0x6700:
      return ESP32_DESFire::LENGTH_ERROR;
    case 0x6982:
      return ESP32_DESFire::SECURITY_NOT_SATISFIED;
    case 0x6985:
      return ESP32_DESFire::CONDITIONS_NOT_SATISFIED;
    case 0x6A82:
      return ESP32_DESFire::FILE_OR_APP_NOT_FOUND;
    case 0x6A86:
      return ESP32_DESFire::INCORRECT_PARAMS;
    case 0x6A87:
      return ESP32_DESFire::INCORRECT_LC;
    case 0x6A00:
      return ESP32_DESFire::CLA_NOT_SUPPORTED;
    case 0x910B:
      return ESP32_DESFire::COMMAND_NOT_FOUND;
    case 0x910C:
      return ESP32_DESFire::COMMAND_FORMAT_ERROR;
    case 0x911C:
      return ESP32_DESFire::ILLEGAL_COMMAND_CODE;
    case 0x911E:
      return ESP32_DESFire::INTEGRITY_ERROR;
    case 0x9140:
      return ESP32_DESFire::NO_SUCH_KEY;
    case 0x917E:
      return ESP32_DESFire::LENGTH_ERROR;
    case 0x919D:
      return ESP32_DESFire::PERMISSION_DENIED;
    case 0x919E:
      return ESP32_DESFire::PARAMETER_ERROR;
    case 0x91AD:
      return ESP32_DESFire::AUTHENTICATION_DELAY;
    case 0x91AE:
      return ESP32_DESFire::AUTHENTICATION_ERROR;
    case 0x91AF:
      return ESP32_DESFire::ADDITIONAL_FRAME;
    case 0x91BE:
      return ESP32_DESFire::BOUNDARY_ERROR;
    case 0x91CA:
      return ESP32_DESFire::COMMAND_ABORTED;
    case 0x91EE:
      return ESP32_DESFire::MEMORY_ERROR;
    case 0x91F0:
      return ESP32_DESFire::FILE_NOT_FOUND;
    case 0x91DE:
      return ESP32_DESFire::DUPLICATE_ERROR;
    case 0x910E:
      return ESP32_DESFire::OUT_OF_EEPROM_ERROR;
    default:
      return ESP32_DESFire::DF_UNKNOWN_ERROR;
  }
}

/////////////////////////////////////////////////////////////////////////////////////
//
// Internal management
//
/////////////////////////////////////////////////////////////////////////////////////

void ESP32_DESFire::printHex(byte* buffer, uint16_t bufferSize) {
  for (uint16_t i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
