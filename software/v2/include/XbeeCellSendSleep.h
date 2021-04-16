#pragma once
#include "XbeeCell.h"

const bool ENTER_CELL_SLEEP = true;
const bool EXIT_CELL_SLEEP = false;

class XbeeCellSendSleep
{
    public:
        XbeeCellSendSleep(XbeeCell *cell, uint32_t sleeprq_out_pin, uint32_t sleep_in_pin);
        bool isModuleOn();
        bool isModuleSleeping();
        void printSleepState();
        bool enterExitSleep(bool state);
        bool wakeSendDataSleep(String data);
    private:
        void pd(String s);
        XbeeCell *cell;
        uint32_t sleeprq_out_pin;       // pin used to request sleep (output to main processor)
        uint32_t sleep_in_pin;          // pin used to show that the module is sleeping (input to main processor)
}; // end