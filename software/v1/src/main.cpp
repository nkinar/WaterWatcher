#include <Arduino.h>
#include <Wire.h>
#include <Cmd.h>
#include "WDTZero.h"
#include "temperature1w.h"
#include "constants.h"
#include "main.h"
#include "gpio.h"
#include "data_storage.h"
#include "cli.h"
#include "flash_mem.h"
#include "spi_local.h"
#include "sdlib.h"
#include "sd_storage.h"
#include "cellular.h"
#include "experiment.h"

/*
WaterWatcher Code
N. J. Kinar (Smart Water Systems Lab)

REFERENCES:
[1] https://www.teachmemicro.com/arduino-turbidity-sensor/
[2] https://www.geeksforgeeks.org/program-for-mean-and-median-of-an-unsorted-array/
[3] https://wiki.dfrobot.com/Gravity__Analog_TDS_Sensor___Meter_For_Arduino_SKU__SEN0244
[4] https://wiki.dfrobot.com/Waterproof_DS18B20_Digital_Temperature_Sensor__SKU_DFR0198_
[5] https://create.arduino.cc/projecthub/TheGadgetBoy/ds18b20-digital-temperature-sensor-and-arduino-9cc806
[6] https://www.arduino.cc/reference/en/language/functions/communication/serial/
[7] http://arduiniana.org/libraries/tinygpsplus/
[8] https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/

NOTES:
1. On the MKR boards, pins 13(RX), 14(TX) are the hardware serial port and this is exposed as an
object Serial1.

2. The default serial port object is Serial that will print to the USB serial port.

3. The hardware interrupt is attached to D4 and this is the RTC interrupt.

4. For the device to send back the data, note that the Hologram.io key needs to be set.  The send_cell variable needs to
be turned on as well for the cellular modem to be sent.

COMMANDS:
print-coeff
key []
sendcell-on
powersave-on
set-name []
write-flash

CMD >> print-coeff
COEFFICIENTS:
valid:1
m:1
alarm_on:0
name:julia
key: []
shutdown_rails:1
send_cell:1
powersave:1
DONE

5.  A watchdog ensures that the entire system is reset within 2 minutes if something goes wrong.

6. For 2 A*hr batteries, the charging time is approximately 5 hours.

*/


//--------------------------------------------
WDTZero watchdog;
//--------------------------------------------

static struct main_data
{
  int serial_port;    // sets the serial port to print
}md;


/*
Set the serial port that needs to be printed
*/
void set_serial_main(int port)
{
  md.serial_port = port;
}

/*
Function to print to the serial port
*/
void printSerial(String s)
{
  if ( md.serial_port ==  USB_STREAM) Serial.println(s);  
  else if (md.serial_port == CELL_STREAM) cellular_send(s);
  else Serial.println(s);  // default to USB
} // end


/*
Print to the serial without line endings
*/ 
void printSerialWithoutLineEnding(String s)
{
  if ( md.serial_port == USB_STREAM) Serial.print(s);
  else if (md.serial_port ==  CELL_STREAM) cellular_send(s);
  else Serial.print(s);  // default to USB
} // end

//---------------------------------------------

void print_data_debug()
{
   print_all_sensors_debug(); 
} // end


void get_print_data_debug()
{
  print_data_debug();
} // end


/*
Function to print the info to a string
*/ 
void print_info()
{
  printSerial(CREDIT_STR " " __DATE__ " " __TIME__);
} // end


void shutdown_func()
{
  #ifdef DEBUG_WATCHDOG
    printSerial("WATCHDOG BARKED");
  #endif
} // end


/*
 * Setup function 
 */
void setup() 
{
  // USB serial port as the default
  md.serial_port = USB_STREAM;

  // setup the watchdog
  watchdog.attachShutdown(shutdown_func);
  watchdog.setup(WDT_SOFTCYCLE16M); // 17 minute watchdog

  // Setup I2C (required before using any I2C libraries)
  Wire.begin();

  // Setup the CLI
  Serial.begin(BAUD_RATE_DEBUG);
  setup_cmd();

  // Setup the serial port for the GPS
  Serial1.begin(BAUD_RATE_GPS);
  while(!Serial1);    // this command might be relatively benign and so it can be kept here
  Serial1.setTimeout(SERIAL_PORT_TIMEOUT);
  
  // rest of the setups
  setup_gpio();
  setup_commands();
  set_rtc_defaults();
  read_flash_and_setup();
  setup_sd();
  set_startup_setup_sd();
  setup_experiment();
} // end


/*
Main loop of the function
*/
void loop()
{
  watchdog.clear();
  gps_run_poll();
  check_rtc();
  poll_cmd();
  timer_update();
} // end 

