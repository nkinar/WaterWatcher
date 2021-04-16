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
void send_data_cell(); 
void format_data_json();
void start_experiment();