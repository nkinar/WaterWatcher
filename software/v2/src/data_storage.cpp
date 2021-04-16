#include <Arduino.h>
#include <stdio.h>
#include "data_storage.h"
#include  "constants.h"
#include "data_storage.h"
#include "gpio.h"
#include "turbidity.h"
#include "temperature1w.h"
#include "tds.h"
#include "main_local.h"
#include "MaximUniqueSerial.h"
#include "PCA9524Reader.h"
#include "BitFun.h"
#include "DS3231.h"
#include "flash_mem.h"
#include "DS2438.h"
#include "GPS.h"
#include "experiment.h"
#include "temperature1w.h"
#include "WaterWatcherOptions.h"


//---------------------------------------------------------------------------------
// Objects
//---------------------------------------------------------------------------------
MaximUniqueSerial uniqueSerial(SERIAL_NUMBER_ADDR);
PCA9534Reader pioReader(PORT_EXP_ADDR);
DS3231 rtc(RTC_ADDR);
DS2438 bmon((uint8_t)ONE_WIRE_BAT_PIN, BMON_SENSE_RESISTOR, BMON_PP);
GPS gps(&Serial1);  // GPS connected to hardware serial port

// struct that holds all of the data
struct main_data_storage ds;

//---------------------------------------------------------------------------------
// FUNCTIONS THAT SAMPLE DATA
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
// EDIT THIS FUNCTION TO ADD ADDITIONAL SENSORS AND VOLTAGES
//---------------------------------------------------------------------------------
void populate_data_first()
{ 
    WaterWatcherOptions *opt = get_options();

    ds.start_time_str = get_time(); 
    if(opt->is_sample_a0()) get_turbidity(ds.a0_voltage); 
    if(opt->is_sample_a1()) get_tds(ds.a1_voltage);
    if(opt->is_sample_a2()) obtain_a2(ds.a2_voltage); 
    bool temp_good;
    get_water_temperature(ds.water_temperature, temp_good);  
    if(temp_good==false) ds.water_temperature = NO_SAMPLE_VALUE;     
} // end


//---------------------------------------------------------------------------------
// FUNCTION TO OBTAIN ANCILLARY DATA
// EDIT THIS FUNCTION TO OBTAIN ADDITIONAL ANCILLARY DATA
//---------------------------------------------------------------------------------
void populate_data_second()
{
  get_serial_number();                                // serial number of unit
  get_state_battery();                                // battery state
  obtain_bmon();                                      // battery monitor information
  get_rtc_temperature();                              // RTC temperature 

  gps_obtain_data();                                  // GPS obtain data
  ds.end_time_str = get_time();                       // ending time of operations
} // end


//---------------------------------------------------------------------------------

// This function returns the sampled data
void get_main_data_struct(struct main_data_storage &ds_out)
{
  ds_out = ds;
} // end


//---------------------------------------------------------------------------------

void get_state_battery()
{
  ds.pio_expander_state = pioReader.readState();

  // #FAULT is low if there is a battery fault
  if(checkBit(ds.pio_expander_state, 0)) ds.battery_fault = false;
  else ds.battery_fault = true;

  // #CHRG is low if the battery is charging
  if(checkBit(ds.pio_expander_state, 1)) ds.battery_charging = false;
  else ds.battery_charging = true;

} // end


void print_state_battery()
{
  get_state_battery();
  printSerial("Battery fault:"); 
  printSerial(ds.battery_fault ? TRUE_STRING : FALSE_STRING);
  printSerial("Battery charging:"); 
  printSerial(ds.battery_charging ? TRUE_STRING : FALSE_STRING);
  printSerial(String(ds.pio_expander_state, DEC));
} // end


void get_serial_number()
{
  ds.serial_number_good = uniqueSerial.getString(ds.serial_number);
} // end


String obtain_serial_number_string()
{
  String s;
  uniqueSerial.getString(s);
  return s;
} // end


void print_serial_number()
{
  String s;
  bool good = uniqueSerial.getString(s);
  if(good) printSerial(s);
  else printSerial("BAD CRC");
} // end


// Obtain the RTC temperature
void get_rtc_temperature()
{
  float temp = rtc.readTemperature();
  ds.rtc_temperature = temp;
} // end


// Print the RTC temperature
void print_rtc_temperature()
{
  get_rtc_temperature();
  printSerial("RTC Temperature (C):");
  printSerial(String(ds.rtc_temperature));
} // end


/*
Time function with format in dd/mm/yyyy hh:mm:ss
This is a 24-hour clock
*/
void time_function(String in)
{
  char s[MAX_TIME_STR_SIZ];
  in.trim();  // ensure no whitespace
  in.toCharArray(s, MAX_TIME_STR_SIZ);
  int day, month, year, hour, minute, second;
  int ret = sscanf(s,TIME_FORMAT, &day, &month, &year, &hour, &minute, &second);
  if(ret != 6)
  {
    print_time();
    return;
  }
  bool rv = rtc.setTime(day, month, year, hour, minute, second);
  if (rv==false) printSerial(ERROR_STRING);
  else printSerial(SUCCESS_STRING);
} // end


// Function to actually get the time
String get_time(bool with_days)
{
  char s[MAX_TIME_STR_SIZ];
  const String wdays[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

  int day, month, year, hour, minute, second, dayNum;
  rtc.getTime(day, month, year, hour, minute, second, dayNum);
  sprintf(s, TIME_FORMAT, day, month, year, hour, minute, second);
  String sv = String(s);
  if(with_days)
  {
    if (dayNum < 1 || dayNum > 7) return sv;
    String dayS = wdays[dayNum-1];
    return sv + "," + dayS;
  }
  return sv;
} // end


// Function to obtain the time as ints
void get_time_ints( int &day, int &month, int &year, int &hour, int &minute, int &second, int &dayNum)
{
  rtc.getTime(day, month, year, hour, minute, second, dayNum);
} // end


/*
Function to print the time to the terminal
*/
void print_time()
{
  printSerial(get_time());
} // end


/*
Set the RTC to defaults on startup if the oscillator has been stopped 
*/
void set_rtc_defaults()
{
  rtc.setDefaultIfOscStopped();
} // end


/*
Read the control print
*/ 
void read_status_print()
{
  uint8_t rv = rtc.readStatus();
  printSerial("RTC Status: " + String(static_cast<uint32_t>(rv))); 
} // end


/*
Force the RTC to be set to the default time
*/
void set_default_time_rtc_force()
{
  rtc.setRTCDefaultTime();
} // end


/*
Set the alarm minutely
*/
bool set_alarm_minutely(int m, bool store)
{
  turn_off_alarms_rtc();  // ensure that alarms are initially turned off
  if(m < 0) return false;
  else if (m > MAX_NUM_MINUTES_RTC) return false;
  set_m(m);
  if (store) 
  {
    set_m_flash(m);
    set_alarm_state_flash(ON_STATE);
  }
  rtc.setAlarmOnceMinuteOrSecond(DS3231_ALARM2_ONCE_MINUTE);
  attach_alarm_interrupt();
  return true;
} // end


/*
Function to clear the RTC alarms in the status register
*/ 
void clear_alarms_rtc()
{
  rtc.clearAlarms(); 
} // end 


/*
Function to turn off the RTC alarms
*/
void turn_off_alarms_rtc()
{
  rtc.offAlarms();
  detach_alarm_interrupt();
  set_alarm_state_flash(OFF_STATE);
} // end


/* 
Function to check and see if the alarm is on
*/
bool iao()
{
  return rtc.isAlarmOn() && get_alarm_on();
} // end


/*
Function to print the bmon addresses
*/
void print_bmon_addr()
{
  bmon.findAll();
  String s = bmon.allAddressString();
  printSerial("Battery monitor addresses:");
  printSerial(s);
  } // end


/*
Call this function to obtain the battery monitor information
*/
  void obtain_bmon()
  {
    bmon.findAll();
    Vector<DS2438Info> v;
    bmon.getDataAll(v);
    int siz = v.size();
    if (siz != 1) return;
    DS2438Info info = v[0];  // only one battery monitor
    ds.btemperature = info.temperature;
    ds.bvoltage = info.voltage;
    ds.bcurrent = info.current;
    ds.bcapacity = info.capacity;
    ds.uptime = info.uptime;
  } // end


void obtain_a2(float &v_out)
{
  bmon.findAll();
  Vector<DS2438VAD> v;
  bmon.getDataVAD(v);
  int siz = v.size();
  if (siz != 1) return;
  DS2438VAD data = v[0];
  v_out = data.vad;
} // end


/*
Print the battery monitor information
*/
void print_bmon()
{
  bmon.findAll();
  Vector<DS2438Info> v;
  bmon.getDataAll(v);
  int siz = v.size();
  printSerial("BATTERY MONITOR:");
  for (int k = 0; k < siz; k++)
  {
    DS2438Info info = v[k];
    float temperature, voltage, current, capacity;
    uint32_t uptime;
    String aString = String(info.astring);
    temperature = info.temperature;
    voltage = info.voltage;
    current = info.current;
    capacity = info.capacity;
    uptime = info.uptime;
    printSerial("-----------------------------------");
    printSerial(String("1w Address: ") + aString);
    printSerial("Temperature: " + String(temperature) + " C");
    printSerial("Voltage: " + String(voltage) + " V");
    printSerial("Current: " + String(current, 4) + " A");
    printSerial("Capacity: " + String(capacity, 4) + " A hr"); 
    printSerial("Uptime: " + String(uptime) + " s");
    printSerial("-----------------------------------");
  }
} // end


/*
Run the operation as per the RTC.   This is the starting point of the sample.
*/
void take_sample()
{
    start_experiment();
} // end


/*
Get the RTC 
*/ 
void get_time_rtc(int &day, int &month, int &year, int &hour, int &minute, int &second, int &dayNum)
{
  rtc.getTime(day, month, year, hour, minute, second, dayNum); 
} // end


/*
Get the RTC time using C language binding
*/
void get_time_rtc_cbind(int *day, int *month, int *year, int *hour, int *minute, int *second, int *dayNum)
{
  rtc.getTime(*day, *month, *year, *hour, *minute, *second, *dayNum); 
} // end


/*
Function to run the poll. 
This is called from the main loop.
*/
void gps_run_poll()
{
  gps.run_poll();
} // end 

/*
Function to start the poll
CLI: gps-start-poll
*/
void gps_start_poll()
{
  gps.start_poll();
} // end 

/*
Function to end the poll
CLI: gps-end-poll
*/
void gps_end_poll()
{
  gps.end_poll();
} // end


/*
Check to see if the GPS data is good
*/
bool is_gps_data_good()
{
  return ds.gdata.good;
} // end

/* 
Call this function to obtain data from the GPS
*/ 
void gps_obtain_data()
{
    sample_gps();
    gps.obtain_gps_data(&ds.gdata);
} // end

/*
CLI: print-gps
*/ 
void print_gps()
{
    // actually obtain the GPS data, then...
    gps_obtain_data();

    // ...print the data from the GPS
    printSerial("GPS data:");
    printSerial("lat:");
    printSerial(String(ds.gdata.lat));
    printSerial("lng:");
    printSerial(String(ds.gdata.lng));
    printSerial("speed:");
    printSerial(String(ds.gdata.speed));
    printSerial("alt:");
    printSerial(String(ds.gdata.alt));
    printSerial("height:");
    printSerial(String(ds.gdata.height));
    printSerial("lat_err:");
    printSerial(String(ds.gdata.lat_err));
    printSerial("long_err:");
    printSerial(String(ds.gdata.long_err));
    printSerial("alt_err:");
    printSerial(String(ds.gdata.alt_err));
    printSerial("hours:");
    printSerial(String(ds.gdata.hours));
    printSerial("minutes:");
    printSerial(String(ds.gdata.minutes));
    printSerial("seconds:");
    printSerial(String(ds.gdata.seconds));
    printSerial("microseconds:");
    printSerial(String(ds.gdata.microseconds));
    printSerial("day:");
    printSerial(String(ds.gdata.day));
    printSerial("month:");
    printSerial(String(ds.gdata.month));
    printSerial("year:");
    printSerial(String(ds.gdata.year));
    printSerial("fix_quality:");
    printSerial(String(ds.gdata.fix_quality));
    printSerial("satellites:");
    printSerial(String(ds.gdata.satellites));
    printSerial("total_sats:"); 
    printSerial(String(ds.gdata.total_sats));
} // end

/*
Sample from the GPS
*/
void sample_gps()
{
  gps.read_data();
} // end

