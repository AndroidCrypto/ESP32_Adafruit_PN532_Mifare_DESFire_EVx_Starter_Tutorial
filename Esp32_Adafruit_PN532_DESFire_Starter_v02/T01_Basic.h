void generateAscendingNumbersHelper(byte* data, uint8_t dataSize) {
  for (byte i = 0; i < dataSize; i++)
    data[i] = i + 1;
}

void run_T01_Basic_Handling() {
  Serial.println();
  Serial.println(DIVIDER);
  Serial.println(" T01 Basic Handling");
  Serial.println(DIVIDER);
  delay(100);
  Serial.println(DIVIDER);

  Serial.println(DIVIDER);
  Serial.println("Get Free Memory");
  appData = new byte[128];
  memset(appData, 0, 128);
  appLenExt = 128;
  appLen = 3;
  dfStatusCode = desfire.DF_Plain_GetFreeMemory(appData, &appLen);
  desfire.DF_StatusCodeDebugPrint(dfStatusCode);
  Serial.printf("GetFreeMemory response length %0d data:", appLen);
  printHex(appData, appLen);
  Serial.println();

  Serial.println(DIVIDER);
  Serial.println("Get Version");
  memset(appData, 0, 128);
  appLenExt = 128;
  appLen = 128;
  byte appLenByte = 128;
  dfStatusCode = desfire.DF_Plain_GetVersion(appData, &appLenByte);
  desfire.DF_StatusCodeDebugPrint(dfStatusCode);
  Serial.printf("GetVersion response length %0d data:", appLenByte);
  printHex(appData, appLenByte);
  Serial.println();

  Serial.println(DIVIDER);
  Serial.println("Create an Application");
  byte aidTutorial[3] = { 0x56, 0x78, 0x9A };
  dfStatusCode = desfire.DF_Plain_CreateApplicationDefaultAes(aidTutorial);
  desfire.DF_StatusCodeDebugPrint(dfStatusCode);

  Serial.println(DIVIDER);
  Serial.println("Create a Standard Data file with Free Access Rights");
  byte sdP01No = 0x01;  // standard data plain
  uint8_t sdP01Size = 32;
  dfStatusCode = desfire.DF_Plain_CreateStandardFileDefaultFreeAccessSized(sdP01No, sdP01Size, ESP32_DESFire::DF_COMMMODE_PLAIN);
  desfire.DF_StatusCodeDebugPrint(dfStatusCode);

  Serial.println(DIVIDER);
  Serial.println("Select an Application");
  dfStatusCode = desfire.DF_Plain_SelectApplication(aidTutorial);
  desfire.DF_StatusCodeDebugPrint(dfStatusCode);

  Serial.println(DIVIDER);
  Serial.println("Create a Standard Data file with Free Access Rights");
  dfStatusCode = desfire.DF_Plain_CreateStandardFileDefaultFreeAccessSized(sdP01No, sdP01Size, ESP32_DESFire::DF_COMMMODE_PLAIN);
  desfire.DF_StatusCodeDebugPrint(dfStatusCode);

  Serial.println(DIVIDER);
  Serial.println("Get File Settings");
  memset(appData, 0, 128);
  appLenByte = 128;
  dfStatusCode = desfire.DF_Plain_GetFileSettings(sdP01No, appData, &appLenByte);
  desfire.DF_StatusCodeDebugPrint(dfStatusCode);
  Serial.println(DIVIDER);
  desfire.DF_FileSettingsDebugPrint();

  Serial.println(DIVIDER);
  Serial.println("Write to a file with Free Access Rights");
  memset(appData, 0, sdP01Size);
  appLenExt = sdP01Size;
  //generateRandomHelper(appData, sdP01Size);
  generateAscendingNumbersHelper(appData, sdP01Size);
  Serial.printf("Generated data length %0d data:", sdP01Size);
  printHex(appData, sdP01Size);
  Serial.println();
  dfStatusCode = desfire.DF_Plain_WriteData_Simple(sdP01No, sdP01Size, 0, appData);
  desfire.DF_StatusCodeDebugPrint(dfStatusCode);

  Serial.println(DIVIDER);
  Serial.println("Read from a file with Free Access Rights");
  memset(appData, 0, sdP01Size);
  appLenExt = 128;
  dfStatusCode = desfire.DF_Plain_ReadData_Simple(sdP01No, sdP01Size, 0, appData, &appLenExt);
  desfire.DF_StatusCodeDebugPrint(dfStatusCode);
  Serial.printf("Read Data response length %0d data:", appLenExt);
  printHex(appData, appLenExt);
  Serial.println();

  delay(100);
  Serial.println(DIVIDER);
  Serial.println(" T01 Basic Handling END");
  Serial.println(DIVIDER);
  Serial.println();
}