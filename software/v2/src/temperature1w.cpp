#include <Arduino.h>
#include <DallasTemperature.h> 
#include <OneWire.h>
#include "temperature1w.h"
#include "constants.h"
#include "Vector.h"
#include "temperature1w.h"

// Objects used to obtain the water temperature
OneWire oneWire(ONE_WIRE_TEMP_PIN); 
DallasTemperature sensors(&oneWire);
Vector<String> temp_sensor_names;
Vector<float> temp_sensor_tf_out;


Vector<float> get_tf_temperature()
{
  return temp_sensor_tf_out;
} // end

/*
 * Function to obtain the water temperature via the 1-wire temperature sensor.
 * NOTE that this code sets the temperature to a default temperature if the 1-wire device cannot be read.
 * This function is only good for reading the first temperature sensor that is on the wire.
 */
void get_water_temperature(float &temperature_C, bool &temp_good)
{
  setup_temperature();   // will call sensors.begin() as required for 1w communications
  if (1)
  {
    sensors.requestTemperatures();
    temperature_C = sensors.getTempCByIndex(0); 
    temp_good = true;
    if (temperature_C == DEVICE_DISCONNECTED_C) 
    {
        temp_good = false;
    }
  }
} // end 

// Call this function to setup the 1w temperature bus
void setup_temperature()
{
  sensors.begin();
} // end

// Obtain the number of devices on the bus
int get_temperature_device_count()
{
  return sensors.getDeviceCount();
} // end


// Function to populate the names of the temperature sensors
// {t0, t1, t2,...}
void populate_names_temp_sensor()
{
  int n = get_temperature_device_count();
  temp_sensor_names.resize(static_cast<unsigned int>(n));
  temp_sensor_tf_out.resize(static_cast<unsigned int>(n));
  for(int k = 0; k < n; k++)
  {
    temp_sensor_names[k] = TEMP_SENSOR_INITIAL_NAME + String(k);
  }
} // end


//-------------------------------------------------------------------------------------------

// Populate temperature sensor values in the transfer function object. 
// Note that this function is safe to call
// since if the 1w device is disconnected, the getTempCByIndex() function
// will only return DEVICE_DISCONNECTED_C.
// CALL THIS FUNCTION DURING THE SAMPLING PROCEDURE
void populate_values_temp_sensor()
{
  // size_t n = temp_sensor_names.size();
  // setup_temperature();                // call to setup the bus 
  // sensors.requestTemperatures();      // obtain all temperatures
  // float out;                          // output temperature
  // for(size_t k = 0; k < n; k++)
  // {
  //   tf.set_measurement_val(temp_sensor_names[k], sensors.getTempCByIndex(k), false);
  //   tf.evaluate(temp_sensor_names[k], out);
  //   temp_sensor_tf_out[k] = out;
  // }
} // end 

//-------------------------------------------------------------------------------------------

String print_addr(DeviceAddress Thermometer) 
{
  String out = ""; 
   for (uint8_t k = 0; k < 8; k++)
  {
    out += String(Thermometer[k], HEX) + " ";
  }
  return out;
} // end


// Function to obtain the temperature on the bus
// CLI: scan-temperature
String temperature_bus_info()
{
  setup_temperature();  // turn on the 1w bus
  DeviceAddress Thermometer;
  String s = "POSITION/TEMPERATURE(deg C)/ADDRESS\n";
  int n = get_temperature_device_count();
  for(int k = 0; k < n; k++)
  {
    sensors.getAddress(Thermometer, k);
    s += String(k) + "/" + String(sensors.getTempCByIndex(k)) + "/" + print_addr(Thermometer) + "\n";
  }
  return s;
  } // end

