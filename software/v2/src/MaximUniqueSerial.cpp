#include <Arduino.h>
#include <Wire.h>
#include <stdint.h>
#include <stddef.h>
#include "MaximUniqueSerial.h"
#include "main_local.h"

MaximUniqueSerial::MaximUniqueSerial(uint8_t addr)
{
    this->addr = addr;
} // end

bool MaximUniqueSerial::getData()
{
    Wire.beginTransmission(addr); 
    Wire.write(byte(0x00));         // pointer to beginning of ROM
    Wire.endTransmission(false);    // repeated start
    Wire.requestFrom(addr, BYTES_MAX_MAXIM_UNIQUE);
    for (size_t k = 0; k < BYTES_MAX_MAXIM_UNIQUE; k++)
    {
        char c = Wire.read();
        data[k] = (uint8_t)c;
    }
    Wire.endTransmission(); 
    uint8_t ans = (uint8_t)dallasCrc8(data, (unsigned int)(BYTES_MAX_MAXIM_UNIQUE-1));
    if (ans==data[BYTES_MAX_MAXIM_UNIQUE-1]) return true;
    return false;
} // end

/*
Obtain the serial number as a string.
NOTE that this also includes the CRC as the last number.
*/ 
bool MaximUniqueSerial::getString(String &s)
{
    bool good = getData();              // obtain the data before concatenating the string
    if (good==false) return false;      // check to see if the data is good before continuing
    s = "";
    for (size_t k = 0; k < BYTES_MAX_MAXIM_UNIQUE; k++)
    {
        s += String((uint32_t)(data[k])) + " ";
    }
    return true;
}


/*
Maxim CRC
REFERENCE:
https://stackoverflow.com/questions/29214301/ios-how-to-calculate-crc-8-dallas-maxim-of-nsdata 
*/
unsigned char MaximUniqueSerial::dallasCrc8(const unsigned char * data, const unsigned int size)
{
    unsigned char crc = 0;
    for ( unsigned int i = 0; i < size; ++i )
    {
        unsigned char inbyte = data[i];
        for ( unsigned char j = 0; j < 8; ++j )
        {
            unsigned char mix = (crc ^ inbyte) & 0x01;
            crc >>= 1;
            if ( mix ) crc ^= 0x8C;
            inbyte >>= 1;
        }
    }
    return crc;
}



