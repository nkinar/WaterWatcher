#pragma once
#include <Arduino.h>
#include <stdint.h>

 // Table 2: Alarm mask bits
 // ALARM 1
const uint8_t DS3231_ALARM1_ONCE_SECOND = 0b01111;                 
const uint8_t DS3231_ALARM1_SECONDS_MATCH = 0b01110;                 
const uint8_t DS3231_ALARM1_MINUTES_SECONDS_MATCH = 0b01100;
const uint8_t DS3231_ALARM1_HOURS_MINUTES_SECONDS_MATCH = 0b01000;
const uint8_t DS3231_ALARM1_DAY_HOURS_MINUTES_SECONDS_MATCH = 0b00000;
const uint8_t DS3231_ALARM1_DAY_OF_WEEK_HOURS_MINUTES_SECONDS_MATCH = 0b10000;
 // ALARM 2
const uint8_t DS3231_ALARM2_ONCE_MINUTE = 0b00111; 
const uint8_t DS3231_ALARM2_MINUTES_MATCH = 0b00110;
const uint8_t DS3231_ALARM2_HOURS_MINUTES_MATCH = 0b00100;
const uint8_t DS3231_ALARM2_DAY_HOURS_MINUTES_MATCH = 0b0000;
const uint8_t DS3231_ALARM2_DAY_OF_WEEK_HOURS_MINUTES_MATCH = 0b1000;

class DS3231
{
public:
    DS3231(uint8_t addr); 
    float readTemperature();
    bool setTime(int day, int month, int year, int hour, int minute, int second);
    void getTime(int &day, int &month, int &year, int &hour, int &minute, int &second, int &dayNum);
    bool setAlarm(uint8_t code, int day, int month, int hour, int minute, int second, int dayNum, int alarmNum);
    bool setAlarmOnceMinuteOrSecond(uint8_t code);
    bool setRTCDefaultTime();
    bool checkOscStopped();
    void setDefaultIfOscStopped();
    uint8_t readStatus();
    void clearAlarms();
    void offAlarms();
    bool isAlarmOn();
private:
    uint8_t addr;
    void convertToBcd(uint8_t *d, const uint8_t input); 
    void writeReg(uint8_t reg, uint8_t data);
    void writeRegs(uint8_t reg, uint8_t *data, size_t num);
    void readReg(uint8_t reg, uint8_t &data);
    void readRegs(uint8_t reg, uint8_t *data, size_t num);
    float convertTemperature(uint8_t upper, uint8_t lower);
    int dayofweek(int y, int m, int d);
    uint16_t convertToBcdNumber(const uint8_t input);
    uint8_t convertToBcdNumberTrunc(const uint8_t input);
    uint8_t convertToBcdNumberInt(const int input);
    int bcdToDecimal(uint8_t hex);
    int isleap(int year);
    int mthdays(int month, int year);
    bool checkRange(int day, int month, int year, int hour, int minute, int second);
    bool checkMaxDays(int day, int month, int year);
    bool checkInitialRanges(int day, int month, int year, int hour, int minute, int second);
}; // end

