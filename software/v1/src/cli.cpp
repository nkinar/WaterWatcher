#include <Arduino.h>
#include <Cmd.h>
#include "cli.h"
#include "constants.h"
#include "gpio.h"
#include "main.h"
#include "data_storage.h"
#include "flash_mem.h"
#include "spi_local.h"
#include "sd_storage.h"
#include "string_helper.h"
#include "cellular.h"
#include "experiment.h"

// command object used to process commands
Cmd cmd;
Cmd modem_cmd;

// function callable from main loop to poll the command objects
void poll_cmd()
{
    cmd.cmdPoll();
} // end

// setup the command objects
void setup_cmd()
{
    // USB serial port stream
    cmd.cmdInit(&Serial);

    // At this time, there is only one stream, but we can set the stream switch if required later
    // cmd.set_stream_switch(USB_STREAM, set_serial_main);

} // end

void on_5v_cmd(int arg_cnt, char **args)
{
    turn_on_5V_ext();
} // end

void off_5v_cmd(int arg_cnt, char **args)
{
    turn_off_5V_ext();
} // end

void print_serial_cmd(int arg_cnt, char **args)
{
    print_serial_number();   
} // end

void bstate_cmd(int arg_cnt, char **args)
{
    print_state_battery(); 
} // end

void rtc_temp_cmd(int arg_cnt, char **args)
{
    print_rtc_temperature();  
} // end


// time dd/mm/yyyy hh:mm:ss
// time
void rtc_time_cmd(int arg_cnt, char **args)
{
    
    if (arg_cnt != 3)
    {
        print_time();  
        return;
    }
    String first = String(args[1]);
    String second = String(args[2]);
    first.trim();
    second.trim();
    String complete = first + " " + second;
    time_function(complete);  
} // end


void rtc_control_cmd(int arg_cnt, char **args)
{
    read_status_print(); 
} //end 

void rtc_default(int arg_cnt, char **args)
{
    set_default_time_rtc_force();
} // end 


void sam(int arg_cnt, char **args)
{
    if (arg_cnt != 2)
    {
        printSerial(ERROR_STRING);  
    }
    String num = String(args[1]);
    num.trim();
    int m = num.toInt();
    printSerial("Minutes: " + String(m));
    if (set_alarm_minutely(m, STORE_MEM)) printSerial(SUCCESS_STRING);
    else printSerial(ERROR_STRING); 
} // end


// Turn off the alarm 
void oa(int arg_cnt, char **args)
{
    turn_off_alarms_rtc();
    printSerial("ALARM OFF");    
} // end


// ISO
// Check to see if the alarm is on
void iao_command(int arg_cnt, char **args)
{
    if(iao()) printSerial("ALARM ON");
    else printSerial("ALARM OFF");
} // end

// print the coefficients
void print_coeff_cmd(int arg_cnt, char **args)
{
    print_flash();
} // end


// set the name of the station
void set_name_cmd(int arg_cnt, char **args)
{
    if(arg_cnt != 2) 
    {
        printSerial(ERROR_STRING);
        return;
    }
    String name = String(args[1]);
    name.trim();
    if (name.length() > MAX_NUM_CHARS_SENSOR_NAME-1)
    {
        printSerial(ERROR_STRING);
        return;
    }
    set_name(name);
    printSerial(SUCCESS_STRING); 
} // end


// write the flash command
void write_flash_cmd(int arg_cnt, char **args)
{
    write_to_flash();
    printSerial(DONE_STRING); 
} // end


// read the flash command
void read_flash_cmd(int arg_cnt, char **args)
{
    read_from_flash();
    printSerial(DONE_STRING); 
} // end

// memory defaults command
void mem_defaults_cmd(int arg_cnt, char **args)
{
    setup_flash_mem_defaults();
} // end

// obtain the battery monitor addresses
void bmon_addr_cmd(int arg_cnt, char **args)
{
    print_bmon_addr();
} // end

// print the battery monitor data
void print_bmon_cmd(int arg_cnt, char **args)
{
    print_bmon();
} // end

// reset the processor
void reset_cmd(int arg_cnt, char **args)
{
    NVIC_SystemReset(); 
} // end

// Start the GPS polling
void gps_start_poll_cmd(int arg_cnt, char **args)
{
    gps_start_poll();
} // end

// End the GPS polling
void gps_end_poll_cmd(int arg_cnt, char **args)
{
    gps_end_poll();
} // end

// Print the GPS command
void print_gps_cmd(int arg_cnt, char **args)
{
    print_gps();
} // end


// sample from the gps but do not print
void sample_gps_cmd(int arg_cnt, char **args)
{
    sample_gps();
} // end


// turn on the sd card voltage
void on_sd_cmd(int arg_cnt, char **args)
{
    on_sd_card();
    printSerial("SD card voltage is ON");
} // end


// turn off the sd card voltage
void off_sd_cmd(int arg_cnt, char **args)
{
    off_sd_card();
    printSerial("SD card voltage is OFF");
} // end


// ls to list the files on the sd card
void ls_sd_cmd(int arg_cnt, char **args)
{
    printSerial("List of files on the SD card:");
    if(scan_files_cli()) printSerial(SUCCESS_STRING); 
    else printSerial(ERROR_STRING);
} // end


// cat a file on the card
// cat [filename]
void cat_sd_cmd(int arg_cnt, char **args)
{
    if (arg_cnt != 2) 
    {
        printSerial(ERROR_STRING);
        return;
    }
    char *file_name = trimwhitespace(args[1]);
    printSerial("File name:");
    printSerial(String(file_name));
    printSerial("---------------------");
    if (sd_print_file(file_name)) 
    {
        return;
    }
    else printSerial(ERROR_STRING);
} // end


// remove a file on the card
void rm_sd_cmd(int arg_cnt, char **args)
{
    if (arg_cnt != 2) 
    {
        printSerial(ERROR_STRING);
        return;
    }
    char *file_name = trimwhitespace(args[1]);
    printSerial("File name:");
    printSerial(String(file_name));

    if(remove_file(file_name)) printSerial(SUCCESS_STRING);
    else printSerial(ERROR_STRING);
} // end 


// echo characters into a file
// echo [filename] [characters]
void echo_cmd(int arg_cnt, char **args)
{
      if (arg_cnt != 3) 
    {
        printSerial(ERROR_STRING);
        return;
    }
    char *file_name = trimwhitespace(args[1]);
    char *characters = trimwhitespace(args[2]);
    printSerial("File name:");
    printSerial(String(file_name));
    printSerial("Characters:");
    printSerial(String(characters));
    if (sd_write_text_to_file(file_name, characters)) printSerial(SUCCESS_STRING);
    else printSerial(ERROR_STRING);
} // end


void key_cmd(int arg_cnt, char **args)
{
    if (arg_cnt != 2) 
    {
        printSerial(ERROR_STRING);
        return;
    }
    char *key = trimwhitespace(args[1]);
    printSerial("key:");
    printSerial(String(key));
    set_key(key); 
} // end

/*
CLI: cell [text to send] 
*/
void cell_send(int arg_cnt, char **args)
{
    if (arg_cnt != 2) 
    {
        printSerial(ERROR_STRING);
        return;
    }
     char *str = trimwhitespace(args[1]);
     printSerial("string:");
     printSerial(String(str));
     check_cellular(); 
     if(cellular_send(String(str))) printSerial(SUCCESS_STRING);
     else printSerial(ERROR_STRING);
} // end


/*
CLI: on-cell 
*/
void on_cell_cmd(int arg_cnt, char **args)
{
    if(begin_gsm()) printSerial(SUCCESS_STRING);
    else printSerial(ERROR_STRING);

} // end


/*
CLI: off-cell
*/
void off_cell_cmd(int arg_cnt, char **args)
{
    end_gsm();
    printSerial("Cellular modem turned off...");
} // end


/*
Print info to the main menu
*/ 
void info_cmd(int arg_cnt, char **args)
{
    print_info(); 
} // end


/*
Turn on the power save mode
*/
void powersave_on_cmd(int arg_cnt, char **args)
{
    set_shutdown_rails(true);

} // end


/*
Turn off the powersave mode
*/
void powersave_off_cmd(int arg_cnt, char **args)
{
    set_shutdown_rails(false);
} // end


/*
Turn on cellular
*/
void send_cell_on(int arg_cnt, char **args)
{
    set_send_cell(true); 
} // end


/*
Turn off cellular
*/
void send_cell_off(int arg_cnt, char **args)
{
    set_send_cell(false);
} // end


/*
Function to start the experiment
*/
void sample_command(int arg_cnt, char **args)
{
    printSerial("Starting experiment from CLI...");
    start_experiment();
    printSerial(DONE_STRING);
} // end


//-------------------------------------------------------------------


void setup_commands()
{
    cmd.cmdAdd(INFO_CMD, info_cmd);
    cmd.cmdAdd(ON_5V_CMD, on_5v_cmd);
    cmd.cmdAdd(OFF_5V_CMD, off_5v_cmd);
    cmd.cmdAdd(PRINT_SERIAL_CMD, print_serial_cmd);
    cmd.cmdAdd(BSTATE_CMD, bstate_cmd);
    cmd.cmdAdd(RTC_TEMP_CMD, rtc_temp_cmd);
    cmd.cmdAdd(RTC_TIME_CMD, rtc_time_cmd); 
    cmd.cmdAdd(RTC_CTRL_CMD, rtc_control_cmd);
    cmd.cmdAdd(RTC_DEFAULT_CMD, rtc_default);
    cmd.cmdAdd(SAM_CMD, sam);
    cmd.cmdAdd(OA_CMD, oa);
    cmd.cmdAdd(IAO_CMD, iao_command);
    cmd.cmdAdd(PRINT_COEFF_CMD, print_coeff_cmd);
    cmd.cmdAdd(SET_NAME_CMD, set_name_cmd);
    cmd.cmdAdd(WRITE_FLASH_CMD, write_flash_cmd);
    cmd.cmdAdd(READ_FLASH_CMD, read_flash_cmd);
    cmd.cmdAdd(MEM_DEFAULTS_CMD, mem_defaults_cmd);
    cmd.cmdAdd(BMON_ADDR_CMD, bmon_addr_cmd);
    cmd.cmdAdd(PRINT_BMON, print_bmon_cmd);
    cmd.cmdAdd(RESET_CMD, reset_cmd);
    cmd.cmdAdd(GPS_START_POLL_CMD, gps_start_poll_cmd); // do not use
    cmd.cmdAdd(GPS_END_POLL_CMD, gps_end_poll_cmd);     // do not use
    cmd.cmdAdd(PRINT_GPS_CMD, print_gps_cmd);
    cmd.cmdAdd(SAMPLE_GPS_CMD, sample_gps_cmd);         // do not use
    cmd.cmdAdd(ON_SD, on_sd_cmd);
    cmd.cmdAdd(OFF_SD, off_sd_cmd);
    cmd.cmdAdd(LS_SD_CMD, ls_sd_cmd);
    cmd.cmdAdd(CAT_CMD, cat_sd_cmd);
    cmd.cmdAdd(RM_CMD, rm_sd_cmd); 
    cmd.cmdAdd(ECHO_CMD, echo_cmd);
    cmd.cmdAdd(KEY_CMD, key_cmd);  
    cmd.cmdAdd(CELL_SEND_CMD, cell_send);
    cmd.cmdAdd(ON_CELL, on_cell_cmd);
    cmd.cmdAdd(OFF_CELL, off_cell_cmd);
    cmd.cmdAdd(POWERSAVE_ON, powersave_on_cmd);
    cmd.cmdAdd(POWERSAVE_OFF, powersave_off_cmd);
    cmd.cmdAdd(SEND_CELL_ON, send_cell_on);
    cmd.cmdAdd(SEND_CELL_OFF, send_cell_off);
    cmd.cmdAdd(SAMPLE, sample_command);
} // end

//-------------------------------------------------------------------

