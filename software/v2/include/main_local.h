#pragma once
#include <Arduino.h>
#include "WaterWatcherOptions.h"

void set_serial_main(int port);
void printSerial(String s);
void printSerialWithoutLineEnding(String s);
void get_data(); 
void print_data_debug();
void get_print_data_debug();
extern "C" void print_debug_c(char *s);
void print_info();
void loop_local();
void setup_local(WaterWatcherOptions *opt);
WaterWatcherOptions *get_options(); 
