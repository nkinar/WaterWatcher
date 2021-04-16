#include <Arduino.h>
#include <Wire.h>
#include <stdint.h>
#include "DS3231.h"
#include "BitFun.h"


// Constants
const size_t BCD_ARRAY_SIZE = 3;
const size_t REG_NUM_WRITE_TIME = 7;
// Registers
const uint8_t SECONDS_REGISTER = 0x00;
const uint8_t MINUTES_REGISTER = 0x01;
const uint8_t HOUR_REGISTER = 0x02;
const uint8_t DAY_REGISTER = 0x03;
const uint8_t DATE_REGISTER = 0x04;
const uint8_t MONTH_REGISTER = 0x05;
const uint8_t YEAR_REGISTER = 0x06;
const uint8_t ALARM1_SECONDS_REGISTER = 0x07;
const uint8_t ALARM1_MINUTES_REGISTER = 0x08;
const uint8_t ALARM1_HOURS_REGISTER = 0x09;
const uint8_t ALARM1_DAY_DATE_REGISTER = 0xA;
const uint8_t ALARM2_MINUTES_REGISTER = 0xB;
const uint8_t ALARM2_HOURS_REGISTER = 0xC;
const uint8_t ALARM2_DAY_DATE_REGISTER = 0xD;
const uint8_t CONTROL_REGISTER = 0xE;
const uint8_t CONTROL_STATUS_REGISTER = 0xF;
const uint8_t AGING_OFFSET_REGISTER = 0x10;
const uint8_t MSB_TEMPERATURE = 0x11;
const uint8_t LSB_TEMPERATURE = 0x12;

const int CURRENT_YEAR = 2019;

// Constructor
DS3231::DS3231(uint8_t addr)
{
    this->addr = addr;
} // end


// Function to convert to BCD
// REFERENCE: Cypress Semiconductor App Note AN2338
void DS3231::convertToBcd(uint8_t d[BCD_ARRAY_SIZE], const uint8_t input)
{
    uint8_t d1 = input / 10;
    d[0] = input - (d1 * 10);
    d[2] = d1 / 10;
    d[1] = d1 - d[2]*10;
} // end 


// Function to convert to BCD number
uint16_t DS3231::convertToBcdNumber(const uint8_t input)
{
    uint8_t d[BCD_ARRAY_SIZE];
    convertToBcd(d, input);
    uint16_t output = ((uint16_t)d[2] << 8u) | ((uint16_t)d[1] << 4u) | (uint16_t)d[0];
    return output;
} // end


// Truncate the higher bits of the number that are not used
uint8_t DS3231::convertToBcdNumberTrunc(const uint8_t input)
{
    uint8_t output = static_cast<uint8_t>(convertToBcdNumber(input));
    return output;
} // end


// Convert from int to bcd
uint8_t DS3231::convertToBcdNumberInt(const int input)
{
    return convertToBcdNumberTrunc(static_cast<uint8_t>(input));
} // end 


// Convert from BCD to decimal
// REFERENCE: https://stackoverflow.com/questions/28133020/how-to-convert-bcd-to-decimal
int DS3231::bcdToDecimal(uint8_t hex)
{
    int dec = ((hex & 0xF0) >> 4) * 10 + (hex & 0x0F);
    return dec;
} // end

// Function to write to a register
void DS3231::writeReg(uint8_t reg, uint8_t data)
{ 
    writeRegs(reg, &data, 1);
} // end 


// Function to write to multiple registers
void DS3231::writeRegs(uint8_t reg, uint8_t *data, size_t num)
{
       Wire.beginTransmission(addr);
       Wire.write(byte(reg));
       for (size_t k = 0; k < num; k++)
       {
            Wire.write(byte(data[k]));
       }
       Wire.endTransmission();  
} // end


// Function to read one byte from a register
void DS3231::readReg(uint8_t reg, uint8_t &data)
{
    uint8_t rd;
    readRegs(reg, &rd, 1);
    data = rd;
} // end


// Function to read from more than one register (using starting register pointer)
void DS3231::readRegs(uint8_t reg, uint8_t *data, size_t num)
{
    Wire.beginTransmission(addr);       // address 
    Wire.write(byte(reg));              // register pointer
    Wire.endTransmission(false);        // repeated start
    Wire.requestFrom(addr, num);        // request number of bytes
    for (size_t k = 0; k < num; k++)
    {
        char c = Wire.read();
        data[k] = (uint8_t)c;
    }
    Wire.endTransmission();
} // end


float DS3231::convertTemperature(uint8_t upper, uint8_t lower)
{
    uint8_t high = upper;
    uint8_t low = lower;
    bool neg = false;
    if(checkBit(high, 7))
    {
        neg = true;
        clearBit(high, 7);
    }
    uint32_t number = ((uint32_t)high << 2u) | ((uint32_t)low >> 6u);
    float temp = static_cast<float>(number) * 0.25f;  // LSB is 0.25 according to datasheet
    return neg ? -temp : temp;
} // end


// Function to read temperature
float DS3231::readTemperature()
{
    const size_t bytes = 2;
    uint8_t td[bytes];
    readRegs(MSB_TEMPERATURE, td, bytes);
    uint8_t high = td[0];
    uint8_t low = td[1];
    float output = convertTemperature(high, low);
    return output;
} // end


/*
Sakamoto's Algorithm for day of the week
REFERENCE:
https://en.wikipedia.org/wiki/Determination_of_the_day_of_the_week#Sakamoto's_methods
*/
int DS3231::dayofweek(int y, int m, int d)	/* 1 <= m <= 12,  y > 1752 (in the U.K.) */
{
    static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    y -= m < 3;
    return (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
} // end

// REFERENCE: https://stackoverflow.com/questions/39463601/determine-the-number-of-days-in-a-month
int DS3231::isleap(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
} // end

// REFERENCE: https://stackoverflow.com/questions/39463601/determine-the-number-of-days-in-a-month
int DS3231::mthdays(int month, int year)
{
    switch(month)
    {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            return 31;
        case 4:
        case 6:
        case 9:
        case 11:
            return 30;
        case 2:
            if(isleap(year)) return 29;
            return 28;
        default:
            break;
    }
    return 0;
} // end


// Function to obtain the time of the RTC
void DS3231::getTime(int &day, int &month, int &year, int &hour, int &minute, int &second, int &dayNum)
{
    uint8_t data[REG_NUM_WRITE_TIME];
    readRegs(SECONDS_REGISTER, data, REG_NUM_WRITE_TIME);
    second = bcdToDecimal(data[0]);
    minute = bcdToDecimal(data[1]);
    hour = bcdToDecimal(data[2]);
    dayNum = bcdToDecimal(data[3]);
    day = bcdToDecimal(data[4]);
    month = bcdToDecimal(data[5]);
    year = bcdToDecimal(data[6]) + 2000;
} // end 

bool DS3231::checkRange(int day, int month, int year, int hour, int minute, int second)
{
    if (day < 1) return false;
    if (month < 1) return false;
    if (year < CURRENT_YEAR) return false;
    if (hour < 0 ) return false;
    if (minute < 0) return false;
    if (second < 0) return false;

    if(month > 12) return false;
    if(year >= 2100) return false;
    if(hour > 23) return false;
    if(minute > 59) return false;
    if(second > 59) return false;
    return true;
} // end


bool DS3231::checkMaxDays(int day, int month, int year)
{
    int max_days_in_month = mthdays(month, year);  
    if (day > max_days_in_month) return false;  
    return true;
} // end


bool DS3231::checkInitialRanges(int day, int month, int year, int hour, int minute, int second)
{
    return checkMaxDays(day, month, year) && checkRange(day, month, year, hour, minute, second);
} // end 


// Ensure that the RTC is set to defaults
bool DS3231::setRTCDefaultTime()
{
    int day = 1;
    int month = 1;
    int year = CURRENT_YEAR;
    int hour = 0;
    int minute = 0;
    int second = 0;
    return setTime(day, month, year, hour, minute, second);
} // end 


// Function to set the time of the RTC
bool DS3231::setTime(int day, int month, int year, int hour, int minute, int second)
{
    if(checkInitialRanges(day, month, year, hour, minute, second)==false) return false;

    int weekday = dayofweek(year, month, day) + 1;  // day of week (1 to 7)
    int yy = year - 2000;                           // year (0 to 99)

    uint8_t data[REG_NUM_WRITE_TIME];
    data[0] = convertToBcdNumberInt(second);
    data[1] = convertToBcdNumberInt(minute);
    data[2] = convertToBcdNumberInt(hour);
    data[3] = convertToBcdNumberInt(weekday);
    data[4] = convertToBcdNumberInt(day);
    data[5] = convertToBcdNumberInt(month);
    data[6] = convertToBcdNumberInt(yy);

    writeRegs(SECONDS_REGISTER, data, REG_NUM_WRITE_TIME);
    return true;
} // end


// Function to set the alarm once per minute or second
bool DS3231::setAlarmOnceMinuteOrSecond(uint8_t code)
{
    if (code != DS3231_ALARM1_ONCE_SECOND && code != DS3231_ALARM2_ONCE_MINUTE) return false;
    int numAlarm;
    if (code == DS3231_ALARM1_ONCE_SECOND) numAlarm = 1;
    else if (code == DS3231_ALARM2_ONCE_MINUTE) numAlarm = 2;
    int day = 1;
    int month = 1;
    int hour = 0;
    int minute = 0;
    int second = 0;
    int dayNum = 1;
    return setAlarm(code, day, month, hour, minute, second, dayNum, numAlarm);
} // end


// Function to set the RTC alarm
// numAlarm = 1 or 2 for the number of the alarm 
bool DS3231::setAlarm(uint8_t code, int day, int month, int hour, int minute, int second, int dayNum, int numAlarm)
{   
    if (numAlarm < 1 || numAlarm > 2) return false; // number of alarm needs to be valid 
    const int year = CURRENT_YEAR;                  // placeholder year for the check function
    if(checkInitialRanges(day, month, year, hour, minute, second)==false) return false;  // data needs to be valid for ranges
    if(dayNum < 1 || dayNum > 7) return false;  // day number needs to be between 1 and 7

    const size_t ALARM_DATA_NUM = 4;
    uint8_t data[ALARM_DATA_NUM];

    data[0] = convertToBcdNumberInt(second);
    data[1] = convertToBcdNumberInt(minute);
    data[2] = convertToBcdNumberInt(hour);
    if (code == DS3231_ALARM1_DAY_OF_WEEK_HOURS_MINUTES_SECONDS_MATCH || code == DS3231_ALARM2_DAY_OF_WEEK_HOURS_MINUTES_MATCH) 
    {
        data[3] = convertToBcdNumberInt(dayNum);
        setBit(data[3], 6);
    }
    else if (code==DS3231_ALARM1_DAY_HOURS_MINUTES_SECONDS_MATCH || code==DS3231_ALARM2_DAY_HOURS_MINUTES_MATCH) 
    {
        data[3] = convertToBcdNumberInt(day);
    }

    // set the 7th bit in the registers according to the code
    uint8_t cnt = 0;
    for (size_t k = static_cast<size_t>(numAlarm)-1; k < ALARM_DATA_NUM; k++)
    {
        if(checkBit(code, cnt++))
        {
            setBit(data[k], 7);
        }
    }
    
    // assign output by shifting the array pointer and setting the start register and the number of bytes output
    size_t bytes_out;
    uint8_t *data_out;
    uint8_t start_register;
    uint8_t control_reg_bit;
    if (numAlarm==1)    // Alarm #1
    {
        bytes_out = ALARM_DATA_NUM;
        data_out = &data[0];
        start_register = ALARM1_SECONDS_REGISTER;
        control_reg_bit = 0;
    }
    else                // Alarm #2
    {
        bytes_out = ALARM_DATA_NUM - 1;
        data_out = &data[1];
        start_register = ALARM2_MINUTES_REGISTER;
        control_reg_bit = 1;
    }

    // write the registers to the RTC
    writeRegs(start_register, data_out, bytes_out);

    // turn on the alarm by toggling the control bits in the control register
    clearAlarms();                  // ensure that the alarms are clear
    uint8_t cr = 0b00011100;        // ensure valid state of control register (startup)
    setBit(cr, control_reg_bit);    // set the bit to turn on the alarm
    writeReg(CONTROL_REGISTER, cr); // write the control register

    return true;
} // end

void DS3231::offAlarms()
{
    uint8_t cr = 0b00011100; 
    writeReg(CONTROL_REGISTER, cr);
    clearAlarms();  // clear the alarm status bits
} // end


// Read the control status register
uint8_t DS3231::readStatus()
{
    uint8_t data;
    readReg(CONTROL_STATUS_REGISTER, data);
    return data;
} // end


// Check to see if the alarm is on
bool DS3231::isAlarmOn()
{
    uint8_t data;
    readReg(CONTROL_REGISTER, data);
    // check to see if Alarm #1 or #2 is on
    if (checkBit(data, 0) || checkBit(data, 1)) return true; 
    return false;
} // end


// Function to clear the alarms in the status register.
// The alarms are only clear when the bit is clear.
void DS3231::clearAlarms()
{
    uint8_t data_status = readStatus();
    for(int k = 0; k <= 1; k++)
    {
        if (checkBit(data_status, k)) 
        {
            clearBit(data_status, k);
        }
    }
     writeReg(CONTROL_STATUS_REGISTER, data_status); 
} // end 


/*
Check if the oscillator has been stopped
*/
bool DS3231::checkOscStopped()
{
    uint8_t data = readStatus();
    if (checkBit(data, 7)) return true;
    return false;
} // end


/*
Set default only if the oscillator has been stopped
*/
void DS3231::setDefaultIfOscStopped()
{
    if (checkOscStopped())
    {
        // set the default time
        setRTCDefaultTime();
        uint8_t data = readStatus();
        // clear the bit showing that the oscillator has stopped
        clearBit(data, 7);
        writeReg(CONTROL_STATUS_REGISTER, data);
    }
} // end



