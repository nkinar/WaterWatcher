#pragma once
#include <Arduino.h>
#include <Wire.h>

class PCA9534Reader
{
public:
    PCA9534Reader(uint8_t addr);
    uint8_t readState();
private:
    uint8_t addr;
}; 

