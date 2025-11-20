# Adafruit_PN532_modified

This is a **modified version** of the original Adafruit_PN532 library (**version 1.3.4**).

There are two modifications in *Adafruit_PN532.cpp*:

- Change the size of the **PN532_PACKBUFFSIZ** from (old value) 64 to (new value) 255:

```` plaintext
//#define PN532_PACKBUFFSIZ 64  ///< Packet buffer size in bytes
#define PN532_PACKBUFFSIZ 255   ///< Packet buffer size in bytes
````

- Change the **timeout** from (old value) 1000 milliseconds  to (new value) 5000 milliseconds in the inDataExchange method:

```` plaintext
bool Adafruit_PN532::inDataExchange(uint8_t *send, uint8_t sendLength,
                                    uint8_t *response,
                                    uint8_t *responseLength) {
...
//if (!sendCommandCheckAck(pn532_packetbuffer, sendLength + 2, 1000)) {
if (!sendCommandCheckAck(pn532_packetbuffer, sendLength + 2, 5000)) {
...
````

Both changes are neccessary to work with larger files and longer processing times.

I just zipped my library and uploaded the zip file.

All credits go to the creator of this library (Adafruit).
