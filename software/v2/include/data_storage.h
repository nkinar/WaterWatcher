#pragma once
#include <Arduino.h>
#include "DS2438.h"
#include "GPS.h"
#include "constants.h"

void get_state_battery(); 
void print_state_battery();
void get_serial_number();
String obtain_serial_number_string();
void print_serial_number(); 
void print_all_sensors_debug(); 
void get_rtc_temperature();
void print_rtc_temperature(); 
void time_function(String in);
void print_time(); 
String get_time(bool with_days=false);
void get_time_ints( int &day, int &month, int &year, int &hour, int &minute, int &second, int &dayNum); 
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
void obtain_a2(float &v_out);

struct main_data_storage
{
    // raw voltages obtained from sensors
    float a0_voltage;           // turbidity
    float a1_voltage;           // tdr
    float a2_voltage;           // external ADC
    float water_temperature;    // water temperature

    // transfer function outputs
    float a0_out;
    float a1_out;
    float a2_out;
    float water_temperature_out;

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

