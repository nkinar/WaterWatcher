#include <Arduino.h>
#include "gpio.h"
#include "constants.h"
#include "data_storage.h"
#include "main_local.h"

struct gpio_data
{
  volatile bool rtc_flag;
  int m;
  bool is_ext_on;
} gd; 


/*
 * Function to turn on the 5V pin
 */
void turn_on_5V_ext()
{
  digitalWrite(EXT_5V_PIN, HIGH);
  gd.is_ext_on = true;
} // end 


/*
 * Function to turn off the 5V pin
 */
void turn_off_5V_ext()
{
  digitalWrite(EXT_5V_PIN, LOW);
  gd.is_ext_on = false;
} // end 


bool check_is_ext_on()
{
  return gd.is_ext_on;
} // end


/* 
Interrupt handler for RTC
*/ 
void handleRTC()
{ 
  gd.rtc_flag = true;  
} // end


// GPIO setup routine
void setup_gpio()
{
  gd.rtc_flag = false;

  // GPIO pins required for system operation
  pinMode(EXT_5V_PIN, OUTPUT);
  pinMode(RTC_INT_PIN, INPUT_PULLUP);
  pinMode(SD_ON, OUTPUT);

  // pins required for operation of the modem
  pinMode(PIN_RX_MODEM, INPUT);
  pinMode(PIN_TX_MODEM, OUTPUT); 
  digitalWrite(PIN_TX_MODEM, 1);            // pin must be high initially
  pinMode(PIN_CTS_MODEM, INPUT);            // CTS is input 
  pinMode(PIN_RESET_MODEM, OUTPUT);
  digitalWrite(PIN_RESET_MODEM, LOW);       // modem reset is initially low on startup (brought high after powerup delay)
  pinMode(SLEEP_PIN_MODEM, INPUT); 
  pinMode(SLEEP_RQ_MODEM, OUTPUT);
  digitalWrite(SLEEP_RQ_MODEM, LOW);

} // end


// Turn on the SD card
void on_sd()
{
   digitalWrite(SD_ON, HIGH);
}


// Turn off the SD card
void off_sd()
{
  digitalWrite(SD_ON, LOW);
}


// Set the alarm GPIO
void attach_alarm_interrupt()
{
  attachInterrupt(digitalPinToInterrupt(RTC_INT_PIN), handleRTC, FALLING);
} // end


// Clear the alarm GPIO
void detach_alarm_interrupt()
{
  detachInterrupt(digitalPinToInterrupt(RTC_INT_PIN));
} // end


/*
Set the m as required for the alarm
*/
void set_m(int m)
{
  gd.m = m;
} // end 


/*
Check whether we need to take the sample or not
*/
void check_take_sample()
{
  int day, month, year, hour, minute, second, dayNum;
  get_time_rtc(day, month, year, hour, minute, second, dayNum);
  if (minute % gd.m == 0) take_sample();
} // end


/*
Function to check if an interrupt has occurred.
This function is called from the main loop.
*/ 
void check_rtc()
{
  if (gd.rtc_flag==false) return;
  gd.rtc_flag = false;
  int val = digitalRead(RTC_INT_PIN);
  if (val==HIGH) return;
  check_take_sample();
  clear_alarms_rtc(); 
} // end
