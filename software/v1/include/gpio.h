#pragma once
void turn_on_5V_ext();
void turn_off_5V_ext();
void setup_gpio();
void check_rtc();
void set_m(int m);
void attach_alarm_interrupt();
void detach_alarm_interrupt(); 
void check_take_sample();
void on_sd();
void off_sd();
bool check_is_ext_on();

