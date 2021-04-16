#include <Arduino.h>
#include <string.h>
#include "cellular.h"
#include "secrets.h"
#include "constants.h"
#include "flash_mem.h"
#include "main.h"
#include <MKRGSM.h>


// When DEBUG_GSM = true, the modem commands wil be printed to the terminal
#ifdef DEBUG_CELLULAR 
    const bool DEBUG_GSM = true;
#else
    const bool DEBUG_GSM = false;
#endif 


GSMClient client;
GPRS gprs;
GSM gsmAccess(DEBUG_GSM);
GSMBand band;
char server[] = "cloudsocket.hologram.io";
int port = 9999;
String HOLOGRAM_TOPIC = CELL_TOPIC;


/*
Cellular data struct
*/
static struct cell_data
{
    bool connected;
    char buffer[CELL_BUFFER_SIZ]; 
    int bpos;
    bool send_good;
    bool client_active; 
} cd;


/*
Call this function to startup the modem before sending a string.
This takes approximately 17 seconds to turn everything on and to join 
CLI: on-cell
*/
bool begin_gsm()
{   
    // https://github.com/arduino-libraries/MKRGSM/issues/66
    // https://github.com/arduino-libraries/MKRGSM/issues/66#issuecomment-577761460
    gprs.setTimeout(20000);       // important to ensure that the modem does not hang
    gsmAccess.setTimeout(20000);  // in ms
    // band.setBand(GSM_MODE_UMTS);  // make sure we are using 3G
    cd.connected = false;
    cd.client_active = false;
    int retries = 3;
    bool flag = false;
    for(int k = 0; k < retries; k++)
    {
        if(gsmAccess.begin() == GSM_READY)
        {
            flag = true;
            break;
        }
    }
    if(flag)
    {
        for(int k = 0; k < retries; k++)
        {
            if(gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD) == GPRS_READY)
            {
                cd.connected = true;
                #ifdef DEBUG_CELLULAR
                    printSerial("cd connected = " + String(cd.connected)); 
                    printSerial("band: " + band.getBand());
                #endif
                return true;
            }
        }
    }
    return false;
} // end


//--------------------------------------------
// Call GSM functions first....
//--------------------------------------------
bool begin_gsm_async()
{
    gsmAccess.begin((const char*)0, true, false);
    return true;
} // end

bool is_gsm_alive()
{
    return gsmAccess.isAccessAlive() ? true: false;
} // end

bool check_gsm_async()
{
    return gsmAccess.ready() == 1;
} // end

//--------------------------------------------
// Call the GPRS functions after....
//--------------------------------------------
bool begin_gprs_async()
{
    gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD, false);
    return true;
} // end

bool check_gprs_async()
{
    return gprs.ready() == 1;
} // end

void set_cell_connected(bool state)
{
    cd.connected = state;
} // end


bool get_cell_connected()
{
    if(cd.connected)
    {
        if (gsmAccess.isAccessAlive() && gprs.status() == GPRS_READY)
        return true;
    }
    return false;
} // end

//--------------------------------------------
// GSM Shutdown (call after all operations)
// If gprs.ready() == 1, the operation is still running
//--------------------------------------------

void gprs_detach_async()
{
    gprs.detachGPRS(false);
} // end

void stop_client()
{
    client.stop();
} // end

void gsm_shutdown()
{
    gsmAccess.shutdown();
} // end

//--------------------------------------------

/*
Call this function to stop the modem in a synchronous fashion.
This takes approximately 4 seconds to turn off the modem and the function will block.
CLI: off-cell
*/ 
void end_gsm()
{
    gprs.detachGPRS();
    client.stop();
    gsmAccess.shutdown();
    cd.connected = false;
    cd.client_active = false;
} // end


/*
Function to check and see if the cellular device is connected
*/
void check_cellular()
{
    if(cd.connected == false)
    {
        begin_gsm();
    }
} // end 


/*
Function to send a string via cellular and the Hologram network
Note that the modem must be paired on the network before this function is called.
*/
bool cellular_send(String data)
{
    memset(cd.buffer, 0, CELL_BUFFER_SIZ);  // clear the string that holds the server response
    cd.bpos = 0;                            // string response position cleared 
    cd.send_good = false;                   // clear the flag that tracks whether the data has been sent properly
    char *key = get_key();
    String send = data;
    data.replace('\n', ' ');                // remove characters that can cause trouble with the modem 
    data.replace('\r', ' ');
    data.replace('\t',' ');
    String HOLOGRAM_DEVICE_KEY = String(key);
    String HOLOGRAM_MESSAGE = send;
    if(client.connect(server, port))
    {
        if (!client.available() && !client.connected()) return false;
        cd.client_active = true;
        client.println("{\"k\":\"" + HOLOGRAM_DEVICE_KEY +"\",\"d\":\""+ HOLOGRAM_MESSAGE+ "\",\"t\":\""+HOLOGRAM_TOPIC+"\"}");
        return true;
    }
    return false;
} // end


/* 
Parse the cellular response and check to see if the data has been sent properly
*/ 
void parse_cell_resp()
{
    #ifdef DEBUG_CELLULAR
        printSerial("Parsing cell response");
    #endif
    String s = String(cd.buffer);
    if(s == String(CELL_RESP_GOOD))
    {
        cd.send_good = true;
    }
    else
    {
        cd.send_good = false;
    }
} // end 


/*
Function to determine whether the data has been sent back by cell
*/
bool get_whether_cell_send()
{
    return cd.send_good;
} // end


/* 
Call to check the cellular communications and to obtain data
*/
void check_cellular_main()
{
    if(cd.connected==false) return;
    if (client.available() && client.connected()) 
    {
        int c = client.read();
        #ifdef DEBUG_CELLULAR
            printSerial(String((char)c));
        #endif
        if (c > 0)
        {
            if (cd.bpos >= CELL_BUFFER_SIZ) cd.bpos = 0;
            cd.buffer[cd.bpos++] = (char)c;
            if (c == CELL_CLOSING_BRACKET) parse_cell_resp();
        }
    }
    check_cellular_connected();
} // end


/*
Check if the cellular modem is connected
*/
void check_cellular_connected()
{
    if (!client.available() && !client.connected() && cd.connected && cd.client_active)
    {
        client.stop();
        cd.client_active = false;
        cd.connected = false;
    }
} // end

