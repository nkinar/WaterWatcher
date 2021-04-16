#include <Arduino.h>
#include <Cmd.h>
#include "cli.h"
#include "constants.h"
#include "gpio.h"
#include "main_local.h"
#include "data_storage.h"
#include "flash_mem.h"
#include "spi_local.h"
#include "sd_storage.h"
#include "string_helper.h"
#include "cellular.h"
#include "experiment.h"
#include "temperature1w.h"
#include "XbeeCellSendSleep.h"
#include "XbeeCell.h"

// objects that need to be called
extern XbeeCellSendSleep pcell;
extern XbeeCell cell;

// command object used to process commands
Cmd cmd;

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


// Print the GPS command
void print_gps_cmd(int arg_cnt, char **args)
{
    print_gps();
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
    Vector<String> files;
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

// key [key value]
void key_cmd(int arg_cnt, char **args)
{
    if (arg_cnt != 2) 
    {
        printSerial(ERROR_STRING);
        return;
    }
    String key = args[1];
    printSerial("key:");
    printSerial(key);
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

    // TODO:
} // end


/*
CLI: on-cell 
*/
void on_cell_cmd(int arg_cnt, char **args)
{
    // TODO:

} // end


/*
CLI: off-cell
*/
void off_cell_cmd(int arg_cnt, char **args)
{
    // TODO:
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
} // end


/*
Set the sensor number (required to send data to the server over cellular)
This sensor number is used along with the key to determine which sensor is communicating with the server.

set-sensor-num [number]
*/
void set_sensor_num(int arg_cnt, char **args)
{
     if(arg_cnt != 2)
    {
        printSerial(ERROR_STRING);
        return;
    }
    String number = args[1];
    set_num(number.toInt());
    printSerial("Set number:" + String(number.toInt()));
} // end


//-------------------------------------------------------------------
// Extended CLI
//-------------------------------------------------------------------

// Helper function to be called by the functions below
void print_success_failure(bool rv)
{
    if(rv) printSerial(SUCCESS_STRING);
    else printSerial(ERROR_STRING);
} // end


void scan_temperature(int arg_cnt, char **args)
{
    // read the water temperature
    float water_temperature;
    bool temp_good;
    if (!check_is_ext_on()) // ext supply needs to be turned on
    {
        turn_on_5V_ext();
        delay(1500); // a delay is required to allow the 1w temperature sensor to stabilize
    }
    get_water_temperature(water_temperature, temp_good); 
    if(temp_good == false) printSerial(ERROR_STRING); 
    printSerial(String(water_temperature));
} // end


//-------------------------------------------------------------------
// CELLULAR
//-------------------------------------------------------------------

/*
Required to send the cellular data:
set-apn-server [APN] [server] [port number]

NOTE that this has to be saved into flash memory to be able to obtain after a power cycle.
*/ 
void set_cellular_config(int arg_cnt, char **args)
{
    if(arg_cnt != 4)
    {
        printSerial(ERROR_STRING);
        return;
    }
    String apn = String(args[1]);
    String server = String(args[2]);
    apn.trim();
    server.trim();
    unsigned int port = static_cast<unsigned int>(String(args[3]).toInt());
    set_apn_server_port(apn, server, port);
} // end 

//-------------------------------------------------------------------

// Helper function for cellular commands
void print_true_false(bool v)
{
  Serial.println(v ? "true" : "false");
} // end

//-------------------------------------------------------------------

// Enter command mode
void enter_cmd(int arg_cnt, char **args)
{
  pcell.enterExitSleep(EXIT_CELL_SLEEP);
  Serial.println("Entering command mode...");
  bool rv = cell.enterCommandMode();
  print_true_false(rv);
} // end

// Obtain hardware info
void hard_info(int arg_cnt, char **args)
{
  pcell.enterExitSleep(EXIT_CELL_SLEEP);
  Serial.println("Hardware info...");
  print_true_false(cell.printHardwareInfo()); 
} // end

// Obtain SIM info
void sim_info(int arg_cnt, char **args)
{
  pcell.enterExitSleep(EXIT_CELL_SLEEP);
  print_true_false(cell.printSIMInfo());
} // end

// Cellular shutdown (power cycle required)
void sd(int arg_cnt, char **args)
{
  pcell.enterExitSleep(EXIT_CELL_SLEEP);
  print_true_false(cell.shutDown());
} // end

// Access the network
void access_network(int arg_cnt, char **args)
{
    pcell.enterExitSleep(EXIT_CELL_SLEEP);
    String apn, server;
    unsigned int port;
    get_apn_server_port(apn, server, port);
    printSerial(String("APN: ") + apn);
    printSerial(String("Server: " + server));
    printSerial(String("Port: ") + String(port));
    cell.setCachedNetworkInfo(apn, server, port);
    print_true_false(cell.setAccessPointNameAndServerConnect());
} // end

// Check association
void ac(int arg_cnt, char **args)
{
  pcell.enterExitSleep(EXIT_CELL_SLEEP);
  Serial.println(cell.checkAssociationCmd());
} // end

// Obtain the network information
void netinfo(int arg_cnt, char **args)
{
  pcell.enterExitSleep(EXIT_CELL_SLEEP);
  cell.getNetworkInfo();
} // end

// Send test string to the server
void send_test_data(int arg_cnt, char **args)
{
  pcell.enterExitSleep(EXIT_CELL_SLEEP);
  String test = String(TEST_STR);  // send the test string
  String received = cell.sendData(test);
  Serial.println("Received:"); 
  Serial.println(received);
  if (received =="RECEIVED")
  {
    Serial.println("Data has been received");
  }
  else
  {
    Serial.println("Some error has occurred");
  }
} // end

// Print whether the module is sleeping
void print_sleep(int arg_cnt, char **arg)
{
  pcell.printSleepState();
} // end

// Wake up the module, send test string and then sleep
void send_then_sleep(int arg_cnt, char **args)
{
  String test = String(TEST_STR);
  print_true_false(pcell.wakeSendDataSleep(test));
} // end

// Setup sleep mode (call this before saving the mode)
void setup_sleep(int arg_cnt, char **arg)
{
  pcell.enterExitSleep(EXIT_CELL_SLEEP);
  print_true_false(cell.enterCommandSetupSleep());
} // end

// Enter sleep mode
void enter_sleep(int arg_cnt, char **arg)
{
  print_true_false(pcell.enterExitSleep(ENTER_CELL_SLEEP));
} // end

// Exit sleep mode
void exit_sleep(int arg_cnt, char **arg)
{
  print_true_false(pcell.enterExitSleep(EXIT_CELL_SLEEP));
} // end 

// Check to see if we are in sleep mode
void check_sleep(int arg_cnt, char **arg)
{
  pcell.enterExitSleep(EXIT_CELL_SLEEP);
  print_true_false(cell.isModuleSetToSleep());
} // end 

// Save the modem state
void save_state(int arg_cnt, char **arg)
{
  pcell.enterExitSleep(EXIT_CELL_SLEEP);
  print_true_false(cell.saveToMemory());
} // end

// Hard reset of cellular modem
void reset_cell(int arg_cnt, char **arg)
{
    printSerial("Resetting modem..."); 
    digitalWrite(PIN_RESET_MODEM, LOW);
    delay(MODEM_DELAY_STARTUP);
    digitalWrite(PIN_RESET_MODEM, HIGH);
    printSerial("Done reset.");
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
    cmd.cmdAdd(PRINT_GPS_CMD, print_gps_cmd);
    cmd.cmdAdd(ON_SD, on_sd_cmd);
    cmd.cmdAdd(OFF_SD, off_sd_cmd);
    cmd.cmdAdd(LS_SD_CMD, ls_sd_cmd);
    cmd.cmdAdd(CAT_CMD, cat_sd_cmd);
    cmd.cmdAdd(RM_CMD, rm_sd_cmd); 
    cmd.cmdAdd(ECHO_CMD, echo_cmd);
    cmd.cmdAdd(ON_CELL, on_cell_cmd);
    cmd.cmdAdd(OFF_CELL, off_cell_cmd);
    cmd.cmdAdd(POWERSAVE_ON, powersave_on_cmd);
    cmd.cmdAdd(POWERSAVE_OFF, powersave_off_cmd);
    cmd.cmdAdd(SEND_CELL_ON, send_cell_on);
    cmd.cmdAdd(SEND_CELL_OFF, send_cell_off);
    cmd.cmdAdd(SAMPLE, sample_command);
    cmd.cmdAdd(SCAN_TEMPERATURE, scan_temperature);     // scan the 1w temperature bus and find values
    
    // Cellular commands that need to be set for the modem to send data to the server
    cmd.cmdAdd(SET_SENSOR_NUM, set_sensor_num);
    cmd.cmdAdd(SET_APN_SERVER_PORT, set_cellular_config);
    cmd.cmdAdd(KEY_CMD, key_cmd);  
    cmd.cmdAdd(CELL_SEND_CMD, cell_send);

    // cellular commands (some for testing of modem)
    cmd.cmdAdd("cmd-modem", enter_cmd);                 // enter command mode
    cmd.cmdAdd("hw-modem", hard_info);                  // print hardware info
    cmd.cmdAdd("sim-modem", sim_info);                  // print SIM info
    cmd.cmdAdd("sd-modem", sd);                         // shutdown the module (used during development and before cutting power)
    cmd.cmdAdd("an-modem", access_network);             // access the network
    cmd.cmdAdd("ac-modem", ac);                         // check association
    cmd.cmdAdd("ni-modem", netinfo);                    // check network info
    cmd.cmdAdd("send-modem", send_test_data);           // send test data to server

    cmd.cmdAdd("print-sleep", print_sleep);             // print whether the module is sleeping or not
    cmd.cmdAdd("setup-sleep", setup_sleep);             // setup sleep mode
    cmd.cmdAdd("enter-sleep", enter_sleep);             // enter sleep mode
    cmd.cmdAdd("exit-sleep", exit_sleep);               // exit sleep mode
    cmd.cmdAdd("restore-default", exit_sleep);          // restore module defaults
    cmd.cmdAdd("save-state", save_state);               // save the module state to onboard flash memory
    cmd.cmdAdd("check-sleep", check_sleep);             // check to see if the module is set up to sleep
    cmd.cmdAdd("reset-cell", reset_cell);               // reset the cellular modem by toggling the pin
    cmd.cmdAdd("send-then-sleep", send_then_sleep);     // send data then sleep
} // end

//-------------------------------------------------------------------

