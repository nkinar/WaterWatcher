#include <Arduino.h>
#include <OneWire.h>
#include <Vector.h>
#include "DS2438.h"
#include "BitFun.h"
#include "main_local.h"


/*
Constructor for class
pin         = digital pin number to which 1-wire bus is attached
senseR      = sense resistor in ohms
pp          = true if parasite power is used
*/
DS2438::DS2438(uint8_t pin, float senseR, bool pp) : w(pin)
{
    this->pin =  pin;
    this->senseR = senseR;
    this->pp = pp;
} // end


void DS2438::findAll()
{   
    uint8_t a[ADDR_SIZ]; 
    addresses.clear();
    while(1)
    {
        if (!w.search(a))
        {
            w.reset_search();
            return;
        }
        if ( OneWire::crc8( a, 7) != a[7]) continue;
        if (a[0] != DS2438_FAMILY_CODE) continue;
        wAddr new_addr;
        for(size_t k = 0; k < ADDR_SIZ; k++) new_addr.assign(k, a[k]);
        addresses.push_back(new_addr); 
    }
} // end


String DS2438::allAddressString()
{
    String s = "";
    int siz = addresses.size();
    for(int k = 0; k < siz; k++)
    {
         wAddr addr = addresses[k];
        s += addr.getString() + String('\n');
    }
    return s;
} // end


bool DS2438::resetSelectCmd(uint8_t *addr, uint8_t *cmd, size_t cmd_len)
{
    uint8_t rv = w.reset();
    if (rv != 1) return false;
    w.select(addr);
    for(size_t k = 0; k < cmd_len; k++) w.write(cmd[k], pp ? 1 : 0);
    return true;
} // end


bool DS2438::resetSelectCmd1(uint8_t *addr, uint8_t cmd)
{
    return resetSelectCmd(addr, &cmd, 1); 
} // end


bool DS2438::readPage(uint8_t *addr, uint8_t data[PAGE_BYTES])
{
    for(size_t k = 0; k < PAGE_BYTES; k++)
    {
        data[k] = w.read();
    }
    w.reset();
    if ( OneWire::crc8( data, 8) != data[8]) return false;
    return true;
} // end


void DS2438::getDataAll(Vector<DS2438Info> &output)
{
    int siz = addresses.size();
    for (int k = 0; k < siz; k++)
    {
        wAddr a = addresses[k];
        float temperature, voltage, current, capacity;
        uint32_t uptime;
        bool rv = readMainData(temperature, voltage, current, a.get());        
        if (rv==false) break;
        rv = readTimeOperationCapacity(uptime, capacity, a.get());
        if (rv==false) break;

        DS2438Info info;
        info.temperature = temperature;
        info.voltage = voltage;
        info.current = current;
        info.uptime = uptime;
        info.capacity = capacity;
        String aString = a.getString();
        aString.toCharArray(info.astring, CHARS_ADDR);
        output.push_back(info);
    }
} // end 


void DS2438::getDataVAD(Vector<DS2438VAD> &output)
{
    int siz = addresses.size();
    for (int k = 0; k < siz; k++)
    {
        wAddr a = addresses[k];
        float v; 
        bool rv = readVAD(v, a.get());
        if (rv==false) break;
        DS2438VAD out;
        String aString = a.getString();
        aString.toCharArray(out.astring, CHARS_ADDR);
        output.push_back(out);
    }
} // end 


bool DS2438::readVAD(float &v, uint8_t *addr)
{
    const uint8_t adc_external = 0x07;  // 0b0111
    const uint8_t adc_bat = 0x0F;       // 0b1111
    
    uint8_t first_cmd_switch[3] = {0x4E, 0x00, adc_external}; 
    resetSelectCmd(addr, first_cmd_switch, 3);     // switch to external ADC input
    resetSelectCmd1(addr, 0xB4);            // convert voltage

    delay(30);  // an ADC conversion takes ~10 ms, but we wait to ensure that the conversion is completely done

    uint8_t first_cmd[2] = {0xB8, 0x00};    // recall memory
    resetSelectCmd(addr, first_cmd, 2);
    uint8_t second_cmd[2]  = {0xBE, 0x00};  // read memory
    resetSelectCmd(addr, second_cmd, 2);

    uint8_t data[PAGE_BYTES];               // read the page with the data
    bool rv = readPage(addr, data);      

    // reset the state back to battery ADC before checking if everything is okay
    uint8_t first_cmd_switch_bat[3] = {0x4E, 0x00, adc_bat}; 
    resetSelectCmd(addr, first_cmd_switch_bat, 3);

    // If the page read did not work, then return false only after the ADC has been
    // switched back to the battery
    if (rv==false) return false;

    // obtain external voltage
    uint8_t voltage_lsb = data[3];
    uint8_t voltage_msb = data[4];
    uint16_t volt = ((uint16_t)voltage_msb << 8u) | ((uint16_t)voltage_lsb);
    v = 10e-3f * (float)volt;

    return true;
} // end


bool DS2438::readTimeOperationCapacity(uint32_t &uptime, float &capacity, uint8_t *addr)
{
    uint8_t first_cmd[2]  = {0xB8, 0x01};
    resetSelectCmd(addr, first_cmd, 2);

    uint8_t second_cmd[2]  = {0xBE, 0x01};
    resetSelectCmd(addr, second_cmd, 2);

    uint8_t data[PAGE_BYTES];               // read the page
    bool rv = readPage(addr, data);
    if (rv==false) return false;

    // uptime in seconds as 32-bit number
    uptime = 0;
    for(uint8_t k = 0; k < 4u; k++)
    {
        uptime |= (data[k] << (8u*k));
    }
    
    // capacity remaining
    uint8_t ica = data[4];
    capacity = (float)(ica) / (2048.0f * senseR);

    return true;
} // end 


bool DS2438::readMainData(float &temperature, float &voltage, float &current, uint8_t *addr)
{
    resetSelectCmd1(addr, 0x44);            // convert temperature
    resetSelectCmd1(addr, 0xB4);            // convert voltage
    uint8_t first_cmd[2] = {0xB8, 0x00};    // recall memory
    resetSelectCmd(addr, first_cmd, 2);
    uint8_t second_cmd[2]  = {0xBE, 0x00};  // read memory
    resetSelectCmd(addr, second_cmd, 2);

    uint8_t data[PAGE_BYTES];               // read the page
    bool rv = readPage(addr, data);         
    if (rv==false) return false;

    // TEMPERATURE
    uint8_t temp_lsb = data[1];
    uint8_t temp_msb = data[2];
    uint16_t temp = ((uint16_t)temp_msb << 8u) | ((uint16_t)temp_lsb);
    temp >>= 3u;
    bool is_neg = false;
    if(checkBit(temp_msb, 7))
    {
        is_neg = true;
        temp = ((~temp)&0xFFF) + 1u;
    }
    temperature = 0.03125f * (float)(temp);
    if (is_neg) temperature *= -1.0f;

    // VOLTAGE
    uint8_t voltage_lsb = data[3];
    uint8_t voltage_msb = data[4];
    uint16_t volt = ((uint16_t)voltage_msb << 8u) | ((uint16_t)voltage_lsb);
    voltage = 10e-3f * (float)volt;

    // CURRENT
    uint8_t current_lsb = data[5];
    uint8_t current_msb = data[6];
    is_neg = false;
    uint16_t curr = ((uint16_t)current_msb << 8u) | ((uint16_t)current_lsb);
    if(checkBit(current_msb, 7))
    {
        is_neg = true;
        curr = ~curr + 0x01;
    }
    current = (float)curr / (4096.0f * senseR);
    if (is_neg) current *= -1.0f;

    // DONE
    return true;
} // end

