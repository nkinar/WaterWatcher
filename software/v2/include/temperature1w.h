#pragma once
#include <Arduino.h>
#include <Vector.h>

// These two vectors can be accessed outside of this file
extern Vector<String> temp_sensor_names;
extern Vector<float> temp_sensor_values;

void get_water_temperature(float &temperature_C, bool &temp_good);
void setup_temperature();
int get_temperature_device_count();
void populate_names_temp_sensor();
String temperature_bus_info();
void populate_values_temp_sensor();
Vector<float> get_tf_temperature();

