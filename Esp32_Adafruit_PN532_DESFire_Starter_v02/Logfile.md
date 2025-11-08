ESP32 Adafruit PN532 DESFire Starter V02
Found chip PN532
Firmware ver. 1.6
ESP32_DESFire library version: 2
Waiting for an ISO14443A card

Tag number: 1
Found a card!

-------------------------------------------------------------------------
 T01 Basic Handling
-------------------------------------------------------------------------
-------------------------------------------------------------------------
-------------------------------------------------------------------------
Get Free Memory
Send length 5
 90 6E 00 00 00
Recv length 5
 00 14 00 91 00
SUCCESS
GetFreeMemory response length 3 data: 00 14 00
-------------------------------------------------------------------------
Get Version
Send length 5
 90 60 00 00 00
Recv length 9
 04 01 01 33 00 18 05 91 AF
Send length 5
 90 AF 00 00 00
Recv length 9
 04 01 01 03 00 18 05 91 AF
Send length 5
 90 AF 00 00 00
Recv length 16
 04 35 68 DA 05 1A 90 20 82 62 30 30 34 23 91 00
SUCCESS
GetVersion response length 28 data: 04 01 01 33 00 18 05 04 01 01 03 00 18 05 04 35 68 DA 05 1A 90 20 82 62 30 30 34 23
-------------------------------------------------------------------------
Create an Application
Send length 11
 90 CA 00 00 05 56 78 9A 0F 85 00
Recv length 2
 91 00
SUCCESS
-------------------------------------------------------------------------
Create a Standard Data file with Free Access Rights
Send length 13
 90 CD 00 00 07 01 00 EE EE 20 00 00 00
Recv length 2
 91 9D
PERMISSION_DENIED ERROR
-------------------------------------------------------------------------
Select an Application
Send length 9
 90 5A 00 00 03 56 78 9A 00
Recv length 2
 91 00
SUCCESS
-------------------------------------------------------------------------
Create a Standard Data file with Free Access Rights
Send length 13
 90 CD 00 00 07 01 00 EE EE 20 00 00 00
Recv length 2
 91 00
SUCCESS
-------------------------------------------------------------------------
Get File Settings
Send length 7
 90 F5 00 00 01 01 00
Recv length 9
 00 00 EE EE 20 00 00 91 00
SUCCESS
-------------------------------------------------------------------------
File Settings for file 01
File Type         : 00 (Standard Data File)
File Options      : 00
File Comm Mode    : PLAIN
File RW/CAR AccRg : EE
File R/W    AccRg : EE
File Size         : 32
-------------------------------------------------------------------------
Write to a file with Free Access Rights
Generated data length 32 data: 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20
Send length 45
 90 8D 00 00 27 01 00 00 00 20 00 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20 00
Recv length 2
 91 00
SUCCESS
-------------------------------------------------------------------------
Read from a file with Free Access Rights
Send length 13
 90 BD 00 00 07 01 00 00 00 20 00 00 00
Recv length 34
 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20 91 00
SUCCESS
Read Data response length 32 data: 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20
-------------------------------------------------------------------------
 T01 Basic Handling END
-------------------------------------------------------------------------
