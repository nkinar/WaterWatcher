#pragma once
#include <Arduino.h>

const size_t WAIT_TIME_BB = 1;                 // 1 us wait time in loop 
class SimpleBBSerial
{
public:
    SimpleBBSerial(uint32_t PIN_RX, uint32_t PIN_TX, uint32_t baud);
    SimpleBBSerial(uint32_t PIN_RX, uint32_t PIN_TX, uint32_t baud, uint32_t PIN_CTS, uint32_t PIN_RTS, uint32_t TIMEOUT_CTS);
    SimpleBBSerial(uint32_t PIN_RX, uint32_t PIN_TX, uint32_t baud, uint32_t PIN_CTS, uint32_t TIMEOUT_CTS);
    void setBaud(uint32_t b);
    bool sendString(String s);
    String receiveString(size_t timeout_cycles, size_t char_num_max, String end);
    void setCannotReceiveData();
    void setCanReceiveData();
private: 
    void sendSerialChar(uint8_t b);
    uint8_t receiveChar(size_t timeout_cycles);
    bool wait_for_timeout();
uint32_t PIN_RX;
uint32_t PIN_TX;
uint32_t DELAY;
uint32_t DELAY_HM;
uint32_t baud;
bool use_cts;
bool use_rts;
uint32_t PIN_CTS;
uint32_t PIN_RTS;
uint32_t TIMEOUT_CTS;
}; // end
