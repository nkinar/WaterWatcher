#pragma once
#include <Arduino.h>
#include "DS2438.h"
#include "GPS.h"
#include "constants.h"

void get_state_battery(); 
void print_state_battery();
void get_serial_number();
void print_serial_number(); 
void print_all_sensors_debug(); 
void get_rtc_temperature();
void print_rtc_temperature(); 
void time_function(String in);
void print_time(); 
String get_time(bool with_days=false);
void set_rtc_defaults();
void read_status_print();
void set_default_time_rtc_force();
bool set_alarm_minutely(int m, bool store);
void clear_alarms_rtc(); 
void turn_off_alarms_rtc();
bool iao();
void print_bmon_addr();
void print_bmon();
void take_sample();
void get_time_rtc(int &day, int &month, int &year, int &hour, int &minute, int &second, int &dayNum);
extern "C" void get_time_rtc_cbind(int *day, int *month, int *year, int *hour, int *minute, int *second, int *dayNum);
void gps_run_poll(); 
void gps_start_poll();
void gps_end_poll();
void print_gps();
void sample_gps();
void gps_obtain_data();
void obtain_bmon();
void populate_data_first();
void populate_data_second();
bool is_gps_data_good();

struct main_data_storage
{
    // turbidity sensor
    float ntu_voltage;
    float ntu;

    // water temperature
    float T_deg_C;
    bool temp_good;
    
    // TDR sensor
    int adc_tds_samples[TDS_SAMPLES];  // NOT SENT BACK
    float tds_voltage;
    float ppm;

    // Serial Number
    String serial_number;
    bool serial_number_good;

    // battery charger state
    bool battery_fault;
    bool battery_charging;
    uint8_t pio_expander_state;

    // rtc temperature
    float rtc_temperature;

    // gps data struct
    struct last_gps_fix gdata;

    // time
    String start_time_str;
    String end_time_str;

    // battery state
    float btemperature;
    float bvoltage;
    float bcurrent;
    float bcapacity;
    uint32_t uptime;

};  // end

// Call this function to obtain the main data struct
void get_main_data_struct(struct main_data_storage &ds_out);

