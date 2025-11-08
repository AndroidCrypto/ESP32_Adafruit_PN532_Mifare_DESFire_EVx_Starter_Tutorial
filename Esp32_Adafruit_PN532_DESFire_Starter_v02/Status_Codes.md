
  enum DF_StatusCode : byte {
    DF_STATUS_OK = 0, // Success (and 0x9000, 0x9100 - OPERATION_OK / Successful operaton)
    DF_STATUS_ERROR = 1, // Error in communication
    DF_STATUS_COLLISION = 2, // Collission detected
    DF_STATUS_TIMEOUT = 3, // Timeout in communication.
    DF_STATUS_NO_ROOM = 4, // A buffer is not big enough.
    DF_STATUS_INTERNAL_ERROR = 5, // Internal error in the code. Should not happen ;-)
    DF_STATUS_INVALID = 6, // Invalid argument.
    DF_STATUS_CRC_WRONG = 7, // The CRC_A does not match
    
    COMMAND_NOT_FOUND = 8, // (910B) - Status used only in Read_Sig.
    COMMAND_FORMAT_ERROR = 9, // (910C) - Status used only in Read_Sig.
    ILLEGAL_COMMAND_CODE = 10, // (911C) Command code not supported.
    INTEGRITY_ERROR = 11, // (911E) CRC or MAC does not match data. Padding bytes not valid.
    NO_SUCH_KEY = 12, // (9140) Invalid key number specified.
    LENGTH_ERROR = 13, // (6700, 917E) Length of command string invalid.
    PERMISSION_DENIED = 14, // (919D) Current configuration / status does not allow the requested command.
    PARAMETER_ERROR = 15, // (919E) Value of the parameter(s) invalid.
    AUTHENTICATION_DELAY = 16, // (91AD) Currently not allowed to authenticate. Keep trying until full delay is spent.
    AUTHENTICATION_ERROR = 17, // (91AE) Current authentication status does not allow the requested command.
    ADDITIONAL_FRAME = 18, // (91AF) Additionaldata frame is expected to be sent.
    BOUNDARY_ERROR = 19, // (91BE) Attempt to read/write data from/to beyond the file's/record's limits. Attempt to exceed the limits of a value file.
    COMMAND_ABORTED = 20, // (91CA) Previous Command was not fully completed. Not all Frames were requested or provided by the PCD.
    MEMORY_ERROR = 21, // (6581, 91EE) Failure when reading or writing to non-volatile memory.
    FILE_NOT_FOUND = 22, // (91F0) Specified file number does not exist.
    SECURITY_NOT_SATISFIED = 23, // (6982) Security status not satisfied.
    CONDITIONS_NOT_SATISFIED = 24, // (6985) Conditions of use not satisfied.
    FILE_OR_APP_NOT_FOUND = 25, // (6A82) File or application not found.
    INCORRECT_PARAMS = 26, // (6A86) Incorrect parameters P1-P2.
    INCORRECT_LC = 27, // (6A87) Lc inconsistent with parameters P1-P2.
    CLA_NOT_SUPPORTED = 28, // (6E00) CLA not supported
    
    DF_WRONG_RESPONSE_LEN = 29,
    DF_WRONG_RESPONSE_CMAC = 30,
    DF_WRONG_RNDA = 31,
    DF_CMD_CTR_OVERFLOW = 32,
    DF_UNKNOWN_ERROR = 33,
    DF_SDM_NOT_IMPLEMENTED_IN_LIB = 34,
    DUPLICATE_ERROR = 35, // (91DE) File is existing (try to create a new one)
    OUT_OF_EEPROM_ERROR = 36,  // (910E) on application or file creation: no more memory available
    DF_STATUS_MIFARE_NACK = 0xff // A MIFARE PICC responded with NAK.
  };
  
  
  enum DF_File : byte {
    DF_FILE_CC = 0x01,
    DF_FILE_NDEF = 0x02,
    DF_FILE_PROPRIETARY = 0x03
  };
  
  
  enum DF_CommMode : byte {
    DF_COMMMODE_PLAIN = 0x00, // 0b00, but can be 0b10 (0x02) as well
    DF_COMMMODE_MAC = 0x01, // 0b01
    DF_COMMMODE_FULL = 0x03 // 0b11
  };
  
  