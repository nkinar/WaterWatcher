#pragma once
#include <Arduino.h>
void set_m_flash(int m);
void set_alarm_state_flash(bool state); 
void setup_flash_mem_defaults();
void read_from_flash();
void write_to_flash();
void set_name(String name);
void print_flash();
void read_flash_and_setup();
int get_m(); 
bool get_alarm_on();
String get_sensor_name(); 
char *get_sensor_name_str();
void set_key(String keyname);
char *get_key();
void set_shutdown_rails(bool state);
bool get_shutdown_rails();
void set_send_cell(bool state); 
bool get_send_cell();

