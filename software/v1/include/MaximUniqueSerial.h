#pragma once
#include <Arduino.h>
#include <stdint.h>

const static size_t BYTES_MAX_MAXIM_UNIQUE = 8;

class MaximUniqueSerial
{
public:
    MaximUniqueSerial(uint8_t addr);
    bool getString(String &s);

private:
    bool getData();
    unsigned char dallasCrc8(const unsigned char * data, const unsigned int size);
    uint8_t addr;
    uint8_t data[BYTES_MAX_MAXIM_UNIQUE];
};


