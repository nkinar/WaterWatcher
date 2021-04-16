#include <Arduino.h>
#include <FlashStorage.h>
#include "flash_mem.h"
#include "main_local.h"
#include "constants.h"
#include "data_storage.h"
#include "safe_string.h"
#include "XbeeCell.h"

/*
Store variables in the microcontroller flash

REFERENCE:
https://github.com/cmaglie/FlashStorage
*/ 

extern XbeeCell cell;

typedef struct
{
    uint8_t send_cell;                                  // 1 to send cellular at the end of the experiment
    uint8_t shutdown_rails_after_rtc_sample;            // 1 to shutdown the rails
    uint8_t valid;                                      // 1 if the data is valid
    uint8_t alarm_on;                                   // 1 if the alarm is on
    uint8_t m;                                          // sampling interval
    uint8_t num;                                        // number of the sensor to be sent to the server
    unsigned int server_port;                           // holds the server port when to contact the server address

    char name[MAX_NUM_CHARS_SENSOR_NAME];               // name of the device
    char key[MAX_NUM_CHARS_KEY_NAME];                   // key to be used

    char apn_addr[CELL_SIZ_CHAR];                       // holds the apn address
    char server_addr[CELL_SIZ_CHAR];                    // holds the server address

} FlashData;

FlashData fm;
FlashStorage(flash_store, FlashData);
static bool flash_good = false;


/*
Function to set the server, APN and port
*/
void set_apn_server_port(String apn, String server, unsigned int port)
{
    apn.toCharArray(fm.apn_addr, CELL_SIZ_CHAR);
    server.toCharArray(fm.server_addr, CELL_SIZ_CHAR);
    fm.server_port = port;
    
    printSerial("APN:" + String(fm.apn_addr));
    printSerial("Server:" + String(fm.server_addr));
    printSerial("Port:" + String(fm.server_port));

    // set the cached network info in the cellular module
    cell.setCachedNetworkInfo(String(fm.apn_addr), String(fm.server_addr), fm.server_port);
} // end


void set_num(int n)
{
    fm.num = n;
} // end


void set_send_cell(bool state)
{
    fm.send_cell = state ? 1 : 0;
} // end


/*
Function to shutdown all rails after sampling to save power
*/
void set_shutdown_rails(bool state)
{
    fm.shutdown_rails_after_rtc_sample = state ? 1 : 0;
} // end


/*
Set the m value in the flash 
*/
void set_m_flash(int m)
{
    fm.m = m;
} // end


/*
Set the alarm state in the flash variables
*/
void set_alarm_state_flash(bool state)
{
    fm.alarm_on = state ? 1: 0;
} // end

/*
Function to setup the flash memory to defaults
CLI: mem-defaults
*/ 
void setup_flash_mem_defaults()
{
    fm.send_cell = 0;
    fm.shutdown_rails_after_rtc_sample = 0;
    fm.valid = 1;
    fm.alarm_on = 0;
    fm.m = 1;
    fm.num = 1;
    strcpy(fm.name, DEFAULT_NAME_SENSOR); 
} // end


/*
Read the variables from flash 
CLI: read-flash 
*/
void read_from_flash()
{
    fm = flash_store.read();
    if (fm.valid==0)
    {
        setup_flash_mem_defaults();
    }
    else
    {
        cell.setCachedNetworkInfo(String(fm.apn_addr), String(fm.server_addr), fm.server_port);
    }
} // end


/*
Write the variables to flash
CLI: write-flash
*/
void write_to_flash()
{
    // physically write the data to flash
    printSerial("Writing data to flash...");
    flash_store.write(fm);
    printSerial(DONE_STRING);
} // end


/*
Set the name of the sensor
CLI: set-name [name]
*/
void set_name(String name)
{
    name.trim();
    name.toCharArray(fm.name, MAX_NUM_CHARS_SENSOR_NAME);
} // end


/*
CLI: key [key] 
*/
void set_key(String keyname)
{
    keyname.trim();
    keyname.toCharArray(fm.key, MAX_NUM_CHARS_KEY_NAME);
} // end


/*
Print the variables from the flash
CLI: print-coeff
*/
void print_flash()
{
    printSerial("COEFFICIENTS:");
    printSerial("valid: " + String(fm.valid)); 
    printSerial("m: " + String(fm.m));
    printSerial("alarm_on: " + String(fm.alarm_on));
    printSerial("name: " + String(fm.name));
    printSerial("key: " + String(fm.key));                        // required to communicate with the server
    printSerial("num: " + String(fm.num));                        // required to communicate with the server
    printSerial("APN: " + String(fm.apn_addr));                   // required by the cellular modem
    printSerial("Server: " + String(fm.server_addr));             // required by the cellular modem
    printSerial("Server Port: " + String(fm.server_port));        // required by the cellular modem
    printSerial("send_cell: " + String(fm.send_cell));               // required by the cellular modem
    printSerial("powersave: " + String(fm.shutdown_rails_after_rtc_sample));
    printSerial("DONE");
} // end


//------------------------------------------------
// SETUP
//------------------------------------------------
void read_flash_and_setup()
{
    read_from_flash();
    if (fm.alarm_on)
    {
        // For some reason, we need to STORE_MEM again for this to work properly
        set_alarm_minutely(fm.m, STORE_MEM);
    }
    else
    {
        turn_off_alarms_rtc();
    }
} // end


//------------------------------------------------
// GETTERS
//------------------------------------------------
bool get_send_cell()
{
    return fm.send_cell ? true: false;
} // end



bool get_shutdown_rails()
{
    return fm.shutdown_rails_after_rtc_sample ? true: false;
} // end


int get_m()
{
    return fm.m;
} // end


bool get_alarm_on()
{
    return fm.alarm_on ? true : false;
} // end


String get_sensor_name()
{
    return (String(fm.name));
} // end


char *get_sensor_name_str()
{
    return fm.name;
} // end 


char *get_key()
{
    return fm.key;
} // end


int get_num()
{
    return fm.num;
} // end


void get_apn_server_port(String &apn, String &server, unsigned int &port)
{
    apn = String(fm.apn_addr);
    server = String(fm.server_addr);
    port = fm.server_port;
} // end

