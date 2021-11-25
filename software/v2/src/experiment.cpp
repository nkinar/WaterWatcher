#include <Arduino.h>
#include <Timer.h>
#include "experiment.h"
#include "main_local.h"
#include "flash_mem.h"
#include "gpio.h"
#include "data_storage.h"
#include "cellular.h"
#include "sd_storage.h"
#include  "spi_local.h"
#include "constants.h"
#include "jWrite.h"
#include "safe_string.h"
#include "temperature1w.h"
#include "constants.h"

//----------------------------------------------------------------------------------------

static Timer t;                                 // timer to wait for the initial turn on 
static struct main_data_storage d;              // struct to hold the data
static String js;                               // string from last observation as JSON
static String csv;                              // string to hold CSV

// Buffers used for copying
static char buff[MAX_BUFF_SIZ_JSON];
static char small_buff[SMALL_BUFF_JSON_SIZ];

// Experiment data used to establish state
struct experiment_data
{
    bool is_running;
    int id;
} ed; // end


void printSerialDebugCell(String s)
{
    #ifdef DEBUG_CELLULAR
        printSerial(s);
    #endif
} // end


/* 
Function to be called from the main loop
*/ 
void timer_update()
{
    t.update();
} // end


/*
Function to setup the experiment
*/
void setup_experiment()
{
    ed.is_running = false;
} // end


/*
Function to clear the experiment
*/
void clear_experiment()
{
    setup_experiment();
} // end


/*
Function to stop the experiment
*/
void stop_experiment()
{
    // Turn off the SD card.  This is always necessary to ensure that the card is periodically reset after each write.
    off_sd_card();

    if (get_shutdown_rails())
    {
        printSerialDebugCell("Shutting down 5V rail if GPS fix is good...");
        // ensure that the GPS has a fix, if not then keep on the 5V rail so that the GPS can get a fix
        if(is_gps_data_good()) 
        {
            printSerialDebugCell("GPS data is good, turning off the 5V rail...");
            turn_off_5V_ext();
        }
    }
    clear_experiment();
} // end


/*
Function to start the experiment
CLI: sample
This is the function that is triggered by the RTC time clock
*/
void start_experiment()
{
    printSerialDebugCell("Starting the experiment...");
    clear_experiment();
    ed.is_running = true;
    if(!check_is_ext_on())
    {
        printSerialDebugCell("Turning on the 5V rail and then waiting...");
        turn_on_5V_ext();
        ed.id = t.after(EXPERIMENT_TICK, start_experiment_second_stage_timer);
    }
    else
    {
        printSerialDebugCell("Not turning on 5V rail, continuing to second stage");
        start_experiment_second_stage();
    }
} // end


/*
Function to populate the first and second data
*/
void populate_first_and_second_data()
{
    populate_data_first();
    populate_data_second();
} // end 


/*
Function to start the second stage and also stop the timer used to insert a delay 
*/ 
void start_experiment_second_stage_timer()
{
    printSerialDebugCell("In second stage for timer");
    t.stop(ed.id);
    start_experiment_second_stage();
} // end


/*
Function to start the second stage of the sampling.
In this function, the data is sent over cellular and also cached to SD card.
*/
void start_experiment_second_stage()
{
    printSerialDebugCell("Starting the second stage, reading data and then GPS");
    for(int i = 0; i < GPS_RETRIES; i++)
    {
        printSerialDebugCell("i = " + String(i));
        populate_first_and_second_data();           // populate the data that is geolocated with GPS measurements
        if (is_gps_data_good()) 
        {
            printSerialDebugCell("GPS data is good...");
            break;
        }
    }
    printSerialDebugCell("Obtaining main data");
    get_main_data_struct(d);  // obtain the data from the main routines
    printSerialDebugCell("Obtained main data");

    // format the data to be sent to SD card and cellular
    format_data_for_storage_and_send();
} // end


// Format the SD card data for storage
void format_data_for_storage_and_send()
{
    String data_json, data_json_additional;
    data_json = format_data_json();

    printSerial("Writing text to file...");
    write_text_to_obs_file(data_json.c_str());
    printSerial("Done writing text to file.");

    //--------------------------------------------
    // Send the data to cellular (if required)
    //--------------------------------------------
    if (!get_send_cell())
    {
        stop_experiment();   
    }
    else
    {
        // the experiment needs to be stopped in the send_data_cell() function
        send_data_cell(data_json);  
    }
} // end


void assemble_string_float(String &s, const float var)
{
    String value = String(var, DEC_PLACES_PRINT);
    s += value + ",";
} // end


template <class T>
void assemble_string_other(String &s, const T var)
{
    String value = String(var);
    s += value + ",";
} // end 


// String copy function from const char to char
char *string_cpy(String s)
{
	strlcpy(small_buff, s.c_str(), SMALL_BUFF_JSON_SIZ);
	return small_buff;
} // end


// Function to check for nan since the nan cannot be placed into JSON string
float check_nan(float n)
{
  if(isnan(n)) return NAN_VALUE_REPLACE; 
  return n; 
} // end


// Format the local data in JSON
String format_data_json()
{
    // Format all of the data
    printSerialDebugCell("Formatting the data...");

    // obtain the options
    WaterWatcherOptions *opt = get_options();

    //--------------------------------------
    // Assign the sampled data
    //--------------------------------------
    if(opt->is_sample_a0())
    {
        opt->set_a0_raw(d.a0_voltage);
    }
    if(opt->is_sample_a1())
    {
        opt->set_a1_raw(d.a1_voltage);
    }
    if(opt->is_sample_a2())
    {
        opt->set_a1_raw(d.a2_voltage);
    }
    if(opt->is_sample_temp0())
    {
        opt->set_temp0_raw(d.water_temperature);
    }

    //--------------------------------------
    // Obtain the transfer function outputs
    // This is done after assignment since 
    // the transfer functions are dependent on the data
    // being collected.
    //--------------------------------------
    if(opt->is_sample_a0())
    {
        d.a0_out = opt->get_a0_out();
    }
    if(opt->is_sample_a1())
    {
        d.a1_out = opt->get_a1_out();
    }
     if(opt->is_sample_a2())
     {
         d.a2_out = opt->get_a2_out();
     }
     if(opt->is_sample_temp0())
     {
         d.water_temperature_out = opt->get_temp0_out();
     }

    //--------------------------------------
    // open the JSON object 
    //--------------------------------------
    jwOpen(buff, MAX_BUFF_SIZ_JSON, JW_OBJECT, JW_COMPACT);

    //-----------------------------------------------------------------
    // TOKEN KEY AND NUMBER (REQUIRED FOR SERVER COMMUNICATIONS)
    //-----------------------------------------------------------------
    jwObj_string("token", get_key());
    jwObj_int("num", get_num());

    //--------------------------------------
    // place the outputs into JSON
    //--------------------------------------
    if(opt->is_sample_a0())
    {
        jwObj_double(string_cpy("a0_voltage"), d.a0_voltage);
        jwObj_double(string_cpy("a0_out"), d.a0_out);
    }
    if(opt->is_sample_a1())
    {
        jwObj_double(string_cpy("a1_voltage"), d.a1_voltage);
        jwObj_double(string_cpy("a1_out"), d.a1_out);
    }
    if(opt->is_sample_a2())
    {
        jwObj_double(string_cpy("a2_voltage"), d.a2_voltage);
        jwObj_double(string_cpy("a2_out"), d.a2_out);
    }
    if(opt->is_sample_temp0())
    {
       jwObj_double(string_cpy("temp0"), d.water_temperature);
       jwObj_double(string_cpy("temp0_out"), d.water_temperature_out);  
    }

    //-----------------------------------------------------------------
    // ADDITIONAL OUTPUTS
    //-----------------------------------------------------------------
    jwObj_string("serial_number", string_cpy(d.serial_number));
    jwObj_bool("serial_number_good", d.serial_number_good);
    jwObj_bool("battery_fault", d.battery_fault);
    jwObj_bool("battery_charging", d.battery_charging);
    jwObj_double("rtc_temperature", check_nan(d.rtc_temperature));
    jwObj_string("start_time_str", string_cpy(d.start_time_str));
    jwObj_string("end_time_str", string_cpy(d.end_time_str));
    jwObj_double("btemperature", check_nan(d.btemperature)); 
    jwObj_double("bvoltage", check_nan(d.bvoltage));
    jwObj_double("bcurrent", check_nan(d.bcurrent));
    jwObj_double("bcapacity", check_nan(d.bcapacity));
    jwObj_int("uptime", d.uptime);
    jwObj_string("name", get_sensor_name_str());

    jwObj_double("latitude", check_nan(d.gdata.lat));
    jwObj_double("longitude", check_nan(d.gdata.lng));
    jwObj_double("speed", check_nan(d.gdata.speed));
    jwObj_double("altitude", check_nan(d.gdata.alt));
    jwObj_double("height", check_nan(d.gdata.height));
    jwObj_double("lat_err", check_nan(d.gdata.lat_err));
    jwObj_double("long_err", check_nan(d.gdata.long_err));
    jwObj_double("alt_err", check_nan(d.gdata.alt_err));
    jwObj_int("gps_hours", d.gdata.hours);
    jwObj_int("gps_minutes", d.gdata.minutes);
    jwObj_int("gps_seconds", d.gdata.seconds);
    jwObj_int("gps_microseconds", d.gdata.microseconds);
    jwObj_int("gps_day", d.gdata.day);
    jwObj_int("gps_month", d.gdata.month);
    jwObj_int("gps_year", d.gdata.year);
    jwObj_int("gps_fix_quality", d.gdata.fix_quality);
    jwObj_int("gps_satellites", d.gdata.satellites);
    jwObj_int("gps_total_sats", d.gdata.total_sats);
    jwObj_bool("gps_gdata_good", d.gdata.good);

    jwClose();                          // close the JSON string (required)

    String out = String(buff);          // convert the JSON data to string (may contain nan)

    printSerialDebugCell("--------DONE JSON-------");
    printSerialDebugCell(out); 
    printSerialDebugCell("-----DONE print JSON--------");
    return out;
} // end


/*
Function to save the data to an SD card
*/
void save_data_to_sd()
{   
    printSerialDebugCell("Writing to observation file"); 
    String swrite = "[" + js + "]" + "\r\n";
    bool rv = write_text_to_obs_file(swrite.c_str());
    printSerialDebugCell("DONE writing to observation file");
    printSerialDebugCell("rv = " + String(rv));
} // end


/*
Send the data via cellular.
*/
void send_data_cell(String s)
{
    send_to_server_over_cellular(s);
    stop_experiment();
} // end


