#include <Arduino.h>
#include "XbeeCellSendSleep.h"
#include "cell_responses.h"

const uint32_t SLEEP_WAIT_POLLS = 30;
const uint32_t SLEEP_WAIT_POLL_TIME_MS = 1000;  // ms
const uint32_t DELAY_EXIT_COMMAND_MODE = 1000;  // ms


void XbeeCellSendSleep::pd(String s)
{
    #ifdef XBEE_CELL_DEBUG
        Serial.println(s);
    #endif
} // end


XbeeCellSendSleep::XbeeCellSendSleep(XbeeCell *cell, uint32_t sleeprq_out_pin, uint32_t sleep_in_pin)
{
    this->cell = cell;
    this->sleeprq_out_pin = sleeprq_out_pin;
    this->sleep_in_pin = sleep_in_pin;
} // end


bool XbeeCellSendSleep::isModuleOn()
{
    return digitalRead(sleep_in_pin);   // high when module is on
} // end


bool XbeeCellSendSleep::isModuleSleeping()
{
    return !isModuleOn();               // true when module is in sleep mode
} // end


void XbeeCellSendSleep::printSleepState()
 {
     if(isModuleSleeping())
     {
         Serial.println("Cell module is sleeping");
     }
     else
     {
         Serial.println("Cell module is ON");
     }
} // end


bool XbeeCellSendSleep::enterExitSleep(bool state)
{
    if(isModuleOn() && (state==EXIT_CELL_SLEEP)) return true; 
    if(isModuleSleeping() && (state==ENTER_CELL_SLEEP)) return true;

    if((state==ENTER_CELL_SLEEP))            
    {
        if(cell->isModuleSetToSleep()) digitalWrite(sleeprq_out_pin, HIGH);
        else return false; // operation did not work since cell module cannot sleep
    }
    else if((state==EXIT_CELL_SLEEP))    
    {
        digitalWrite(sleeprq_out_pin, LOW);
    }
    for(uint32_t k = 0; k < SLEEP_WAIT_POLLS; k++)
    {
        if(state==ENTER_CELL_SLEEP) if(isModuleSleeping()) return true;
        if(state==EXIT_CELL_SLEEP) if(isModuleOn()) return true;
        delay(SLEEP_WAIT_POLL_TIME_MS);
    }
    // if we get here, the exit from sleep state has not been successful and there is a timeout
    return false;
} // end


// Call this function to wake up the module, send data and then sleep
bool XbeeCellSendSleep::wakeSendDataSleep(String data)
{
    bool rv = false;
    String resp;

    pd("Exiting sleep mode");
    enterExitSleep(EXIT_CELL_SLEEP);                // exit sleep mode

    pd("Reconnecting if required");
    rv = cell->reconnectIfRequired();               // reconnect if required to the network (and ensure that command mode is exited)
    if(!rv) goto cleanup;
    delay(DELAY_EXIT_COMMAND_MODE);                 // wait after exiting command mode to ensure that modem can send data

    pd("Sending data");
    resp = cell->sendData(data);                    // send the data and obtain response
    pd("Response: " + resp);
    if (resp != CELL_RECEIVED_STR) rv = false;
    else rv = true;
    delay(DELAY_EXIT_COMMAND_MODE);                 // wait after sending data to ensure that the data has been sent
cleanup:
    pd("Entering sleep mode");
    enterExitSleep(ENTER_CELL_SLEEP);               // enter cell sleep mode
    return rv;
} // end


