#pragma once
#include <Arduino.h>
void setup_experiment(); 
void clear_experiment();
void timer_update();
void check_experiment_state_machine();
void stop_experiment();
void save_data_to_sd();
void start_experiment_second_stage();
void start_experiment_second_stage_timer();
void send_data_cell(String s); 
String format_data_json();
String format_data_csv();
void start_experiment();
void format_data_for_storage_and_send();
float check_nan(float n);

