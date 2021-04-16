#pragma once
#include <Arduino.h>
#include <Vector.h>
#include <OneWire.h>
#include "constants.h"

const size_t ADDR_SIZ = 8;                      // size of the addresses
const uint8_t DS2438_FAMILY_CODE = 0x26;        // family code
const size_t PAGE_BYTES = 9;                    // 8 bytes + 1 CRC byte

class wAddr
{
    public:
        wAddr() 
        {
        }
        void assign(size_t indx, uint8_t data)
        {
            if(indx < ADDR_SIZ)
            {
                addr[indx] = data;
            }
        }
        uint8_t *get()
        {
            return addr;
        }
        String getString()
        {
            String s = "";
            for(size_t k = 0; k < ADDR_SIZ; k++)
            {
                s += String(addr[k], HEX) + " ";
            } 
            s.trim();
            return s;
        }
    private:
        uint8_t addr[ADDR_SIZ];
}; // end

const size_t CHARS_ADDR = 32; 
struct DS2438Info
{
    char astring[CHARS_ADDR]; 
    float temperature; 
    float voltage;
    float current;
    uint32_t uptime;
    float capacity;
    
    float vad_voltage;
}; // end

// struct with the external voltage
struct DS2438VAD
{
    char astring[CHARS_ADDR]; 
    float vad;
}; // end


class DS2438
{
public:
        DS2438(uint8_t pin, float senseR, bool pp);
        void findAll();
        String allAddressString();
        void getDataAll(Vector<DS2438Info> &output);
        void getDataVAD(Vector<DS2438VAD> &output);
        uint8_t getDigitalPin() {return pin;}
private:
    bool resetSelectCmd(uint8_t *addr, uint8_t *cmd, size_t cmd_len);
    bool resetSelectCmd1(uint8_t *addr, uint8_t cmd);
    bool readMainData(float &temperature, float &voltage, float &current, uint8_t *addr);
    bool readTimeOperationCapacity(uint32_t &uptime, float &capacity, uint8_t *addr);
    bool readPage(uint8_t *addr, uint8_t *data);
    bool readVAD(float &v, uint8_t *addr);

    OneWire w;
    Vector<wAddr>addresses;
    
    float senseR;
    bool pp;
    uint8_t pin;
}; // end

