#include <Arduino.h>
#include <string.h>
#include "cellular.h"
#include "constants.h"
#include "flash_mem.h"
#include "main_local.h"
#include "SimpleBBSerial.h"
#include "XbeeCell.h"
#include "XbeeCellSendSleep.h"

// Objects to send cellular data
SimpleBBSerial bbs(PIN_RX_MODEM, PIN_TX_MODEM, BAUD_MODEM, PIN_CTS_MODEM, -1, TIMEOUT_CTS);
XbeeCell cell(&bbs);
XbeeCellSendSleep pcell(&cell, SLEEP_RQ_MODEM, SLEEP_PIN_MODEM);


/*
Call this function to setup the cellular
*/ 
void setup_cellular()
{

} // end


/*
Call this function to send the data to the server via cellular
*/
void send_to_server_over_cellular(String s)
{
    printSerial("---DATA TO BE SENT TO CELLULAR MODEM---");
    printSerial(s);
    printSerial("-----------");
    for(uint8_t k = 0; k < CELLULAR_RETRIES; k++)
    {
       bool rv = pcell.wakeSendDataSleep(s); 
       if(rv==true)
       {
            printSerial(SUCCESS_STRING);
            break;
       }
       else
       {
           printSerial(ERROR_STRING);
       } 
    }
} // end 

