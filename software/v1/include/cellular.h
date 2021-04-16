#pragma once
#include <Arduino.h>

bool begin_gsm(); 
void end_gsm();
void check_cellular();
bool cellular_send(String data);
void check_cellular_main();
void parse_cell_resp();
void stop_client();
bool get_whether_cell_send();

bool begin_gsm_async();
bool check_gsm_async();
bool begin_gprs_async();
bool check_gprs_async();
void set_cell_connected(bool state);
void stop_client();
bool is_gsm_alive();
bool get_cell_connected();
void gprs_detach_async(); 
void stop_client();
void gsm_shutdown();
void check_cellular_connected();
unsigned long get_local_time_cell();
