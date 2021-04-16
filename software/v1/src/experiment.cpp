#include <Arduino.h>
#include <Timer.h>
#include "experiment.h"
#include "main.h"
#include "flash_mem.h"
#include "gpio.h"
#include "data_storage.h"
#include "cellular.h"
#include "sd_storage.h"
#include  "spi_local.h"
#include "constants.h"

//----------------------------------------------------------------------------------------

static Timer t;                                 // timer to wait for the initial turn-on
static struct main_data_storage d;              // struct to hold the data
static String js;                               // string from last observation

struct experiment_data
{
    bool is_running;
    int id;
} ed; 


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
Function to start the sampling
*/
void start_experiment_second_stage()
{
    printSerialDebugCell("Starting the second stage and reading GPS");
    for(int i = 0; i < GPS_RETRIES; i++)
    {
        printSerialDebugCell("i = " + String(i));
        populate_first_and_second_data();
        if (is_gps_data_good()) 
        {
            printSerialDebugCell("GPS data is good...");
            break;
        }
    }
    printSerialDebugCell("Obtaining main data");
    get_main_data_struct(d);  // obtain the data from the main routines
    printSerialDebugCell("Obtained main data");
    format_data_json();
    save_data_to_sd();
    send_data_cell();
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


/*
Format the JSON data before it is saved to an SD card
*/
void format_data_json()
{
    printSerialDebugCell("Obtaining data from the main struct");

    // Format all of the data
    printSerialDebugCell("Formatting the data");
    js = "";
    assemble_string_float(js, d.ntu_voltage); 
    assemble_string_float(js, d.ntu);
    assemble_string_float(js, d.T_deg_C);
    assemble_string_other(js, d.temp_good);
    assemble_string_float(js, d.tds_voltage);
    assemble_string_float(js, d.ppm);
    assemble_string_other(js, d.serial_number);
    assemble_string_other(js, d.serial_number_good);
    assemble_string_other(js, d.battery_fault);
    assemble_string_other(js, d.battery_charging);
    assemble_string_float(js, d.rtc_temperature);
    assemble_string_other(js, d.start_time_str);
    assemble_string_other(js, d.end_time_str);
    assemble_string_float(js, d.btemperature);
    assemble_string_float(js, d.bvoltage);
    assemble_string_float(js, d.bcurrent);
    assemble_string_float(js, d.bcapacity);
    assemble_string_other(js, d.uptime);
    assemble_string_other(js, get_sensor_name());

    // GPS data
    assemble_string_float(js, d.gdata.lat);
    assemble_string_float(js, d.gdata.lng);
    assemble_string_float(js, d.gdata.speed);
    assemble_string_float(js, d.gdata.alt);
    assemble_string_float(js, d.gdata.height);
    assemble_string_float(js, d.gdata.lat_err);
    assemble_string_float(js, d.gdata.long_err);
    assemble_string_float(js, d.gdata.alt_err);
    assemble_string_other(js, d.gdata.hours);
    assemble_string_other(js, d.gdata.minutes);
    assemble_string_other(js, d.gdata.seconds);
    assemble_string_other(js, d.gdata.microseconds);
    assemble_string_other(js, d.gdata.day);
    assemble_string_other(js, d.gdata.month);
    assemble_string_other(js, d.gdata.year);
    assemble_string_other(js, d.gdata.fix_quality);
    assemble_string_other(js, d.gdata.satellites);
    assemble_string_other(js, d.gdata.total_sats);
    assemble_string_other(js, d.gdata.good);

    printSerialDebugCell("--------DONE-------");
    printSerialDebugCell(js); 
    printSerialDebugCell("-----DONE print--------");

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
Send the data via cellular
*/
void send_data_cell()
{
    printSerialDebugCell("In data cellular send"); 
    // check to see if the data needs to be sent via cellular
    if (!get_send_cell())
    {
        printSerialDebugCell("Not sending data, stopping experiment"); 
        stop_experiment();
        return;
    } // end

    printSerialDebugCell("Cellular modem is not connected, beginning connection"); 
    bool rv = begin_gsm();
    printSerialDebugCell(String(rv));
    printSerialDebugCell("Done connection to cellular network"); 
    if (rv==false) return;
  
    printSerialDebugCell("Sending data...");
    printSerialDebugCell(js);
    int retries = 3;
    for(int k = 0; k < retries; k++)
    {
        rv = cellular_send(js); 
        if(rv) break;
    }
    if(rv==false) return;
    int timeout = CELL_SEND_POLL;
    while( get_whether_cell_send() == false && get_cell_connected())
    {
        check_cellular_main();
        --timeout;
        if (timeout <= 0) break;
    }
    if(get_whether_cell_send()==true) 
    {
        printSerialDebugCell("Cell has been sent successfully");
    }
    end_gsm();  // always turn off modem once something has been sent
    // beyond this point, we cannot call end_gsm() since this will cause the micro to hang.
    stop_experiment();
} // end

