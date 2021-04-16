#include <Arduino.h>
#include <DallasTemperature.h> 
#include <OneWire.h>
#include "temperature1w.h"
#include "constants.h"

// Objects used to obtain the water temperature
OneWire oneWire(ONE_WIRE_TEMP_PIN); 
DallasTemperature sensors(&oneWire);

/*
 * Function to obtain the water temperature via the 1-wire temperature sensor
 * NOTE that this code sets the temperature to a default temperature if the 1-wire device cannot be read.
 */
void get_water_temperature(float &temperature_C, bool &temp_good)
{
  setup_temperature(); 
  if (1)
  {
    sensors.requestTemperatures();
    temperature_C = sensors.getTempCByIndex(0); 
    temp_good = true;
    if (temperature_C == DEVICE_DISCONNECTED_C) 
    {
        temperature_C = DEFAULT_TEMPERATURE;
        temp_good = false;
    }
  }
} // end 


void setup_temperature()
{
  sensors.begin();
} // end

