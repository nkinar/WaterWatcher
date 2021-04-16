#include <Arduino.h>
#include "SimpleBBSerial.h"


SimpleBBSerial::SimpleBBSerial(uint32_t PIN_RX, uint32_t PIN_TX, uint32_t baud)
{
    this->PIN_RX = PIN_RX;
    this->PIN_TX = PIN_TX;
    use_cts = false;
    use_rts = false;
    this->baud = baud;
    setBaud(baud);
} // end


SimpleBBSerial::SimpleBBSerial(uint32_t PIN_RX, uint32_t PIN_TX, uint32_t baud, uint32_t PIN_CTS, uint32_t PIN_RTS, uint32_t TIMEOUT_CTS)
{
  use_cts = false;
  use_rts = false;
  if (PIN_CTS != (uint32_t)(-1)) use_cts = true;
  if (PIN_RTS != (uint32_t)(-1)) use_rts = true;
  this->PIN_CTS = PIN_CTS; 
  this->PIN_RTS = PIN_RTS;
  this->PIN_RX = PIN_RX;
  this->PIN_TX = PIN_TX;
  this->TIMEOUT_CTS = TIMEOUT_CTS;
  this->baud = baud;
  setBaud(baud);
} // end 


SimpleBBSerial::SimpleBBSerial(uint32_t PIN_RX, uint32_t PIN_TX, uint32_t baud, uint32_t PIN_CTS, uint32_t TIMEOUT_CTS)
{
  use_cts = true;
  use_rts = false;
  this->PIN_CTS = PIN_CTS; 
  this->PIN_RX = PIN_RX;
  this->PIN_TX = PIN_TX;
  this->TIMEOUT_CTS = TIMEOUT_CTS;
  this->baud = baud;
  setBaud(baud);
} // end 


void SimpleBBSerial::setCannotReceiveData()
{
  if(use_rts) digitalWrite(PIN_RTS, 1);
} // end


void SimpleBBSerial::setCanReceiveData()
{
  if(use_rts) digitalWrite(PIN_RTS, 0);
} // end


void SimpleBBSerial::setBaud(uint32_t b)
{
    this->baud = b;
    float delay = 1.0f/static_cast<float>(b);
    float delay_usf = delay/1.0e-6;
    DELAY = static_cast<uint32_t>(delay_usf);
    DELAY_HM = DELAY + (DELAY/4);
} // end 


void SimpleBBSerial::sendSerialChar(uint8_t b)
{
  digitalWrite(PIN_TX, 0);    // start bit
  delayMicroseconds(DELAY); 
  for(uint8_t k = 0; k < 8; k++)
  {
    if(bitRead(b, k))
    {
      digitalWrite(PIN_TX, 1);
    }
    else
    {
      digitalWrite(PIN_TX, 0);
    }
    delayMicroseconds(DELAY); 
  }
  digitalWrite(PIN_TX, 1);  // stop bit 
  delayMicroseconds(DELAY);
} // end



bool SimpleBBSerial::wait_for_timeout()
{
  if(use_cts)
  {
    if(digitalRead(PIN_CTS) == HIGH)             // remote device cannot accept data
    {
      uint32_t cnt;
      for(cnt = 0; cnt <= TIMEOUT_CTS; cnt++)
      {
        delayMicroseconds(DELAY);
        if(digitalRead(PIN_CTS) == LOW) break;    // device can accept data
      }
      if(cnt == TIMEOUT_CTS) return false;        // timeout occurred, so data cannot be sent
    }
  }
  return true;
} // end



bool SimpleBBSerial::sendString(String s)
{
  unsigned int n = s.length();
  for(unsigned int k = 0; k < n; k++)
  {
    if(!wait_for_timeout()) return false;
    char c = s.charAt(k);
    sendSerialChar(static_cast<uint8_t>(c));
  }
  return true;
} // end


uint8_t SimpleBBSerial::receiveChar(size_t timeout_cycles)
{
  uint8_t out = 0;
  for(size_t t = 0; t < timeout_cycles; t++)
  {
    if(digitalRead(PIN_RX) == 0)  // start bit
    {
      delayMicroseconds(DELAY_HM);
      for(size_t k = 0; k < 8; k++)
      {
        if (digitalRead(PIN_RX))
        {
          bitSet(out, k);
        }
        delayMicroseconds(DELAY);
      }
      if(digitalRead(PIN_RX) == 1)  // stop bit
      {
        return out;
      }
      return 0;
    }
    delayMicroseconds(WAIT_TIME_BB); // delay in loop to wait for timeout
  }
  return out;
} // end


String SimpleBBSerial::receiveString(size_t timeout_cycles, size_t char_num_max, String end)
{
  String input;
  if (timeout_cycles == 0 || char_num_max == 0) return input;
  bool flag = false;
  if(end.length() != 0) flag = true;
  for(size_t n = 0; n < char_num_max; n++)
  {
    uint8_t c = receiveChar(timeout_cycles);
    if (c==0) return input;  // if a NULL char is received, exit the operation
    input += (char)c;
    if(flag && input.endsWith(end))
    {
      return input;
    }
  }
  return input;
} // end

