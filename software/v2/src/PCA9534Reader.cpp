#include <Arduino.h>
#include <Wire.h>
#include "PCA9524Reader.h"

// DEFINITIONS
static const uint8_t INPUT_PORT_REG = 0x00;
static const uint8_t OUTPUT_PORT_REG = 0x01;
static const uint8_t POL_INV_REG = 0x02;
static const uint8_t CONFIG_REG = 0x03;

// Constructor 
PCA9534Reader::PCA9534Reader(uint8_t addr)
{
    this->addr = addr;
} // end 

// State reader
uint8_t PCA9534Reader::readState()
{
    Wire.beginTransmission(addr);
    Wire.write(byte(INPUT_PORT_REG)); 
    Wire.endTransmission(false);    // repeated start
    Wire.requestFrom(addr, 1);
    char c = Wire.read();
    Wire.endTransmission();
    return (uint8_t)c;
} // end