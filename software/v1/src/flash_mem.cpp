#include <Arduino.h>
#include <FlashStorage.h>
#include "flash_mem.h"
#include "main.h"
#include "constants.h"
#include "data_storage.h"

/*
Store variables in the microcontroller flash

REFERENCE:
https://github.com/cmaglie/FlashStorage
*/ 

typedef struct
{
    int send_cell;
    int shutdown_rails_after_rtc_sample;
    int valid;
    int alarm_on;
    int m;
    char name[MAX_NUM_CHARS_SENSOR_NAME];
    char key[MAX_NUM_CHARS_KEY_NAME];

} FlashData;

FlashData fm;
FlashStorage(flash_store, FlashData);
static bool flash_good = false;


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
    fm.shutdown_rails_after_rtc_sample = 0;
    fm.valid = 1;
    fm.m = 1;
    fm.alarm_on = 0;
    strncpy(fm.name, DEFAULT_NAME_SENSOR, sizeof(DEFAULT_NAME_SENSOR)); 
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
        flash_good = true;
    }
} // end


/*
Write the variables to flash
CLI: write-flash
*/
void write_to_flash()
{
    flash_store.write(fm);
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
    printSerial("valid:" + String(fm.valid)); 
    printSerial("m:" + String(fm.m));
    printSerial("alarm_on:" + String(fm.alarm_on));
    printSerial("name:" + String(fm.name));
    printSerial("key:" + String(fm.key));
    printSerial("shutdown_rails:" + String(fm.shutdown_rails_after_rtc_sample));
    printSerial("send_cell:" + String(fm.send_cell));
    printSerial("powersave:" + String(fm.shutdown_rails_after_rtc_sample));
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


